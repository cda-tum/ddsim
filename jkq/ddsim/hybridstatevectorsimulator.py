"""Backend for DDSIM Hybrid Schrodinger-Feynman Simulator."""

import logging

from qiskit.providers.models import BackendConfiguration

from jkq import ddsim
from jkq.ddsim.hybridqasmsimulator import HybridQasmSimulator

logger = logging.getLogger(__name__)


class HybridStatevectorSimulator(HybridQasmSimulator):
    """Python interface to JKQ DDSIM Hybrid Schrodinger-Feynman Simulator"""

    SHOW_STATE_VECTOR = True

    def __init__(self, configuration=None, provider=None):
        conf = {
            'backend_name': 'hybrid_statevector_simulator',
            'backend_version': ddsim.__version__,
            'url': 'https://github.com/iic-jku/ddsim',
            'simulator': True,
            'local': True,
            'description': 'JKQ DDSIM C++ simulator',
            'basis_gates': ['id', 'u0', 'u1', 'u2', 'u3', 'cu3',
                            'x', 'cx',
                            'y', 'cy',
                            'z', 'cz',
                            'h', 'ch',
                            's', 'sdg', 't', 'tdg',
                            'rx', 'crx',
                            'ry', 'cry',
                            'rz', 'crz',
                            'p', 'cp', 'cu1',
                            'sx', 'csx', 'sxdg',
                            # 'swap', 'cswap', 'iswap',
                            'snapshot'],
            'memory': False,
            'n_qubits': 64,
            'coupling_map': None,
            'conditional': False,
            'max_shots': 1000000000,
            'open_pulse': False,
            'gates': []
        }
        super().__init__(configuration=configuration or BackendConfiguration.from_dict(conf), provider=provider)

    def _validate(self, quantum_circuit):
        return
