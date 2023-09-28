"""Sampler implementation for an artibtrary Backend object."""

from __future__ import annotations

import math
import uuid
from collections.abc import Sequence
from typing import Any

from qiskit.circuit.quantumcircuit import QuantumCircuit
from qiskit.providers.backend import BackendV1, BackendV2
from qiskit.providers.options import Options
from qiskit.result import QuasiDistribution, Result
from qiskit.transpiler.passmanager import PassManager

from qiskit.primitives.backend_estimator import _prepare_counts
from qiskit.primitives.base import BaseSampler, SamplerResult
from qiskit.primitives.utils import _circuit_key
from qiskit.primitives.primitive_job import PrimitiveJob

from mqt.ddsim.job import DDSIMJob
from mqt.ddsim.qasmsimulator import QasmSimulatorBackend


class DDSIMBackendSampler(BaseSampler[PrimitiveJob[SamplerResult]]):
    """A :class:`~.BaseSampler` implementation that provides an interface for
    leveraging the sampler interface from any backend.

    This class provides a sampler interface from any backend and doesn't do
    any measurement mitigation, it just computes the probability distribution
    from the counts. It facilitates using backends that do not provide a
    native :class:`~.BaseSampler` implementation in places that work with
    :class:`~.BaseSampler`, such as algorithms in :mod:`qiskit.algorithms`
    including :class:`~.qiskit.algorithms.minimum_eigensolvers.SamplingVQE`.
    However, if you're using a provider that has a native implementation of
    :class:`~.BaseSampler`, it is a better choice to leverage that native
    implementation as it will likely include additional optimizations and be
    a more efficient implementation. The generic nature of this class
    precludes doing any provider- or backend-specific optimizations.
    """

    def __init__(
        self,
        backend: BackendV2,
        options: dict | None = None,
        bound_pass_manager: PassManager | None = None,
        skip_transpilation: bool = False,
    ):
        """Initialize a new BackendSampler

        Args:
            backend: Required: the backend to run the sampler primitive on
            options: Default options.
            bound_pass_manager: An optional pass manager to run after
                parameter binding.
            skip_transpilation: If this is set to True the internal compilation
                of the input circuits is skipped and the circuit objects
                will be directly executed when this objected is called.
        Raises:
            ValueError: If backend is not provided
        """

        super().__init__(options=options)
        self._backend = backend
        self._transpile_options = Options()
        self._bound_pass_manager = bound_pass_manager
        self._preprocessed_circuits: list[QuantumCircuit] | None = None
        self._transpiled_circuits: list[QuantumCircuit] = []
        self._skip_transpilation = skip_transpilation
        self._circuit_ids = {}


    @property
    def transpiled_circuits(self) -> list[QuantumCircuit]:
        if self._skip_transpilation:
            self._transpiled_circuits = list(self._circuits)
        elif len(self._transpiled_circuits) < len(self._circuits):
            # transpile only circuits that are not transpiled yet
            self._transpile()
        return self._transpiled_circuits

    @property
    def backend(self) -> BackendV2:
        return self._backend

    @property
    def transpile_options(self) -> Options:
        return self._transpile_options
        
    def set_transpile_options(self, **fields):
        self._transpile_options.update_options(**fields)
        
    def _transpile(self):
        from qiskit import transpile

        start = len(self._transpiled_circuits)
        self._transpiled_circuits.extend(
            transpile(
                self._circuits[start:],
                target = self._backend.target,
                **self.transpile_options.__dict__,
            ),
        )

    def _run(
        self,
        circuits: Sequence[QuantumCircuit],
        parameter_values: Sequence[Parameters],
        **run_options,
    ) -> PrimitiveJob:
    
        circuit_indices = []
        for circuit in circuits:
            index = self._circuit_ids.get(_circuit_key(circuit))
            if index is not None:
                circuit_indices.append(index)
            else:
                circuit_indices.append(len(self._circuits))
                self._circuit_ids[_circuit_key(circuit)] = len(self._circuits)
                self._circuits.append(circuit)
                self._parameters.append(circuit.parameters)
                
                
        job = PrimitiveJob(self._call, circuit_indices, parameter_values, **run_options)
        job.submit()
        return job

    def _call(
        self,
        circuits: Sequence[QuantumCircuit],
        parameter_values: Sequence[Parameters],
        **run_options,
    ) -> SamplerResult:
    
    
        transpiled_circuits = self.transpiled_circuits
        bound_circuits = self.backend._assign_parameters(transpiled_circuits, parameter_values)
        bound_circuits = self._bound_pass_manager_run(bound_circuits)
        
        result = [self.backend.run(bound_circuits, **run_options).result()]
           
        return self._postprocessing(result, bound_circuits)
    

    def _postprocessing(
        self, result: list[Result], circuits: list[QuantumCircuit]
    ) -> SamplerResult:
        counts = _prepare_counts(result)
        shots = sum(counts[0].values())

        probabilities = []
        metadata: list[dict[str, Any]] = [{} for _ in range(len(circuits))]
        for count in counts:
            prob_dist = {k: v / shots for k, v in count.items()}
            probabilities.append(
                QuasiDistribution(prob_dist, shots=shots, stddev_upper_bound=math.sqrt(1 / shots))
            )
            for metadatum in metadata:
                metadatum["shots"] = shots

        return SamplerResult(probabilities, metadata)


    def _bound_pass_manager_run(self, circuits):
        if self._bound_pass_manager is None:
            return circuits
        else:
            output = self._bound_pass_manager.run(circuits)
            if not isinstance(output, list):
                output = [output]
            return output
            
          


 
from qiskit import QuantumCircuit, QuantumRegister, ClassicalRegister
from qiskit.circuit import Parameter
import numpy as np



qc= QuantumCircuit(2)
qc.h(0)
qc.cx(0,1)
qc.measure_all()



sampler= DDSIMBackendSampler(QasmSimulatorBackend())
job =sampler.run([qc])
result = job.result()

print(result.quasi_dists)

