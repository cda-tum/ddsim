#pragma once

#include "CircuitSimulator.hpp"
#include "QuantumComputation.hpp"
#include "StochasticNoiseSimulator.hpp"
#include "dd/NoiseFunctionality.hpp"
#include "dd/Package.hpp"

class DeterministicNoiseSimulator: public CircuitSimulator<dd::DensityMatrixSimulatorDDPackageConfig> {
public:
    DeterministicNoiseSimulator(std::unique_ptr<qc::QuantumComputation>&& qc_,
                                const ApproximationInfo&                  approximationInfo_,
                                const std::string&                        noiseEffects_          = "APD",
                                double                                    noiseProbability_      = 0.001,
                                std::optional<double>                     ampDampingProbability_ = std::nullopt,
                                double                                    multiQubitGateFactor_  = 2):
        CircuitSimulator(std::move(qc_), approximationInfo_),
        noiseEffects(StochasticNoiseSimulator::initializeNoiseEffects(noiseEffects_)),
        noiseProbSingleQubit(noiseProbability_),
        ampDampingProbSingleQubit(ampDampingProbability_ ? ampDampingProbability_.value() : noiseProbability_ * 2),
        noiseProbMultiQubit(noiseProbability_ * multiQubitGateFactor_),
        ampDampingProbMultiQubit(ampDampingProbSingleQubit * multiQubitGateFactor_),
        deterministicNoiseFunctionality(dd,
                                        getNumberOfQubits(),
                                        noiseProbSingleQubit,
                                        noiseProbMultiQubit,
                                        ampDampingProbSingleQubit,
                                        ampDampingProbMultiQubit,
                                        noiseEffects) {
        StochasticNoiseSimulator::sanityCheckOfNoiseProbabilities(noiseProbability_, ampDampingProbSingleQubit, multiQubitGateFactor_);
    }

    explicit DeterministicNoiseSimulator(std::unique_ptr<qc::QuantumComputation>&& qc_,
                                         const std::string&                        noiseEffects_          = "APD",
                                         double                                    noiseProbability_      = 0.001,
                                         std::optional<double>                     ampDampingProbability_ = std::nullopt,
                                         double                                    multiQubitGateFactor_  = 2):
        DeterministicNoiseSimulator(std::move(qc_), {}, noiseEffects_, noiseProbability_, ampDampingProbability_, multiQubitGateFactor_) {}

    DeterministicNoiseSimulator(std::unique_ptr<qc::QuantumComputation>&& qc_,
                                const ApproximationInfo&                  approximationInfo_,
                                const std::size_t                         seed_,
                                const std::string&                        noiseEffects_          = "APD",
                                double                                    noiseProbability_      = 0.001,
                                std::optional<double>                     ampDampingProbability_ = std::nullopt,
                                double                                    multiQubitGateFactor_  = 2):
        CircuitSimulator(std::move(qc_), approximationInfo_, seed_),
        noiseEffects(StochasticNoiseSimulator::initializeNoiseEffects(noiseEffects_)),
        noiseProbSingleQubit(noiseProbability_),
        ampDampingProbSingleQubit(ampDampingProbability_ ? ampDampingProbability_.value() : noiseProbability_ * 2),
        noiseProbMultiQubit(noiseProbability_ * multiQubitGateFactor_),
        ampDampingProbMultiQubit(ampDampingProbSingleQubit * multiQubitGateFactor_),
        deterministicNoiseFunctionality(dd,
                                        getNumberOfQubits(),
                                        noiseProbSingleQubit,
                                        noiseProbMultiQubit,
                                        ampDampingProbSingleQubit,
                                        ampDampingProbMultiQubit,
                                        noiseEffects) {
        StochasticNoiseSimulator::sanityCheckOfNoiseProbabilities(noiseProbability_, ampDampingProbSingleQubit, multiQubitGateFactor_);
    }

    std::map<std::string, std::size_t> measureAllNonCollapsing(std::size_t shots) override {
        return sampleFromProbabilityMap(rootEdge.getSparseProbabilityVectorStrKeys(measurementThreshold), shots);
    }

    void initializeSimulation(std::size_t nQubits) override;
    char measure(dd::Qubit i) override;
    void reset(qc::NonUnitaryOperation* nonUnitaryOp) override;
    void applyOperationToState(std::unique_ptr<qc::Operation>& op) override;

    std::map<std::string, std::size_t> sampleFromProbabilityMap(const dd::SparsePVecStrKeys& resultProbabilityMap, std::size_t shots);

    [[nodiscard]] std::size_t getActiveNodeCount() const override { return Simulator::dd->template getUniqueTable<dd::dNode>().getNumActiveEntries(); }
    [[nodiscard]] std::size_t getMaxNodeCount() const override { return Simulator::dd->template getUniqueTable<dd::dNode>().getPeakNumActiveEntries(); }

    [[nodiscard]] std::size_t countNodesFromRoot() override {
        qc::DensityMatrixDD::alignDensityEdge(rootEdge);
        const std::size_t tmp = rootEdge.size();
        qc::DensityMatrixDD::setDensityMatrixTrue(rootEdge);
        return tmp;
    }

    qc::DensityMatrixDD rootEdge{};

private:
    std::vector<dd::NoiseOperations> noiseEffects;

    double noiseProbSingleQubit{};
    double ampDampingProbSingleQubit{};
    double noiseProbMultiQubit{};
    double ampDampingProbMultiQubit{};

    double                                                                         measurementThreshold = 0.01;
    dd::DeterministicNoiseFunctionality<dd::DensityMatrixSimulatorDDPackageConfig> deterministicNoiseFunctionality;
};
