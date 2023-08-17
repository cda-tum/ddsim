"""Utilities for constructing a DDSIM header."""

from qiskit import QuantumCircuit


class DDSIMHeaderBuilder:
    def __init__(self, name, n_qubits, memory_slots, global_phase, creg_sizes, clbit_labels, qreg_sizes, qubit_labels):
        self.name = name
        self.n_qubits = n_qubits
        self.memory_slots = memory_slots
        self.global_phase = global_phase
        self.creg_sizes = creg_sizes
        self.clbit_labels = clbit_labels
        self.qreg_sizes = qreg_sizes
        self.qubit_labels = qubit_labels

    @classmethod
    def build_header_dict(cls, qc: QuantumCircuit):
        qubit_labels = []
        clbit_labels = []
        qreg_sizes = []
        creg_sizes = []
        for qreg in qc.qregs:
            qreg_sizes.append([qreg.name, qreg.size])
            for j in range(qreg.size):
                qubit_labels.append([qreg.name, j])
        for creg in qc.cregs:
            creg_sizes.append([creg.name, creg.size])
            for j in range(creg.size):
                clbit_labels.append([creg.name, j])

        return {
            "clbit_labels": clbit_labels,
            "qubit_labels": qubit_labels,
            "creg_sizes": creg_sizes,
            "qreg_sizes": qreg_sizes,
            "n_qubits": qc.num_qubits,
            "memory_slots": qc.num_clbits,
            "name": qc.name,
            "global_phase": qc.global_phase,
        }

    @classmethod
    def from_circ(cls, qc: QuantumCircuit):
        data = cls.build_header_dict(qc)
        return cls(**data)
        
    def to_dict(self):   
        header_dict = {
            "clbit_labels": self.clbit_labels,
            "qubit_labels": self.qubit_labels,
            "creg_sizes": self.creg_sizes,
            "qreg_sizes": self.qreg_sizes,
            "n_qubits": self.n_qubits,
            "memory_slots": self.memory_slots,
            "name": self.name,
            "global_phase": self.global_phase,
        }
        return header_dict  
