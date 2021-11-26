import unittest

from qiskit import *

from jkq import ddsim


class JKQStandaloneSimulatorTests(unittest.TestCase):
    def test_standalone(self):
        circ = QuantumCircuit(3)
        circ.h(0)
        circ.cx(0, 1)
        circ.cx(0, 2)

        sim = ddsim.PathCircuitSimulator(circ)
        result = sim.simulate(1000)
        self.assertEqual(len(result.keys()), 2)
        self.assertIn('000', result.keys())
        self.assertIn('111', result.keys())

    def test_standalone_with_seed(self):
        circ = QuantumCircuit(3)
        circ.h(0)
        circ.cx(0, 1)
        circ.cx(0, 2)

        sim = ddsim.PathCircuitSimulator(circ, 1337)
        result = sim.simulate(1000)
        self.assertEqual(len(result.keys()), 2)
        self.assertIn('000', result.keys())
        self.assertIn('111', result.keys())

    def test_standalone_config_object(self):
        circ = QuantumCircuit(3)
        circ.h(0)
        circ.cx(0, 1)
        circ.cx(0, 2)
        mode = ddsim.PathSimulatorMode.bracket
        sim = ddsim.PathCircuitSimulator(circ, seed=0, mode=mode, bracket_size=2, alternating_start=1, nthreads=1)
        result = sim.simulate(1000)
        self.assertEqual(len(result.keys()), 2)
        self.assertIn('000', result.keys())
        self.assertIn('111', result.keys())

    def test_standalone_pairwise(self):
        circ = QuantumCircuit(3)
        circ.h(0)
        circ.cx(0, 1)
        circ.cx(0, 2)
        mode = ddsim.PathSimulatorMode.pairwise_recursive
        sim = ddsim.PathCircuitSimulatorTest(circ, mode)
        result = sim.simulate(1000)
        self.assertEqual(len(result.keys()), 2)
        self.assertIn('000', result.keys())
        self.assertIn('111', result.keys())

    def test_standalone_pairwise_with_seed(self):
        circ = QuantumCircuit(3)
        circ.h(0)
        circ.cx(0, 1)
        circ.cx(0, 2)
        config = ddsim.Configuration()
        sim = ddsim.PathCircuitSimulator(circ, 1337, config)
        result = sim.simulate(1000)
        self.assertEqual(len(result.keys()), 2)
        self.assertIn('000', result.keys())
        self.assertIn('111', result.keys())
