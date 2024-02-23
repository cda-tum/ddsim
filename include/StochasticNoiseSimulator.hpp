#pragma once

#include "CircuitSimulator.hpp"
#include "QuantumComputation.hpp"
#include "dd/NoiseFunctionality.hpp"
#include "nlohmann/json.hpp"

#include <future>
#include <limits>
#include <optional>
#include <taskflow/taskflow.hpp>
#include <thread>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <variant>
#include <vector>

class StochasticNoiseSimulator: public CircuitSimulator<dd::StochasticNoiseSimulatorDDPackageConfig> {
public:
    StochasticNoiseSimulator(std::unique_ptr<qc::QuantumComputation>&& qc_,
                             const ApproximationInfo&                  approximationInfo_,
                             const std::string&                        noiseEffects_          = "APD",
                             double                                    noiseProbability_      = 0.001,
                             std::optional<double>                     ampDampingProbability_ = std::nullopt,
                             double                                    multiQubitGateFactor_  = 2):
        CircuitSimulator(std::move(qc_), approximationInfo_),
        noiseProbability(noiseProbability_),
        amplitudeDampingProb((ampDampingProbability_) ? ampDampingProbability_.value() : noiseProbability_ * 2),
        multiQubitGateFactor(multiQubitGateFactor_),
        maxInstances(std::thread::hardware_concurrency() > 4 ? std::thread::hardware_concurrency() - 4 : 1),
        noiseEffects(initializeNoiseEffects(noiseEffects_)) {
        sanityCheckOfNoiseProbabilities(noiseProbability_, amplitudeDampingProb, multiQubitGateFactor_);
    }

    explicit StochasticNoiseSimulator(std::unique_ptr<qc::QuantumComputation>&& qc_,
                                      const std::string&                        noiseEffects_          = "APD",
                                      double                                    noiseProbability_      = 0.001,
                                      std::optional<double>                     ampDampingProbability_ = std::nullopt,
                                      double                                    multiQubitGateFactor_  = 2):
        StochasticNoiseSimulator(std::move(qc_), {}, noiseEffects_, noiseProbability_, ampDampingProbability_, multiQubitGateFactor_) {}

    StochasticNoiseSimulator(std::unique_ptr<qc::QuantumComputation>&& qc_,
                             const ApproximationInfo&                  approximationInfo_,
                             const std::size_t                         seed_,
                             const std::string&                        noiseEffects_          = "APD",
                             double                                    noiseProbability_      = 0.001,
                             std::optional<double>                     ampDampingProbability_ = std::nullopt,
                             double                                    multiQubitGateFactor_  = 2):
        CircuitSimulator(std::move(qc_), approximationInfo_, seed_),
        noiseProbability(noiseProbability_),
        amplitudeDampingProb((ampDampingProbability_) ? ampDampingProbability_.value() : noiseProbability_ * 2),
        multiQubitGateFactor(multiQubitGateFactor_),
        maxInstances(std::thread::hardware_concurrency() > 4 ? std::thread::hardware_concurrency() - 4 : 1),
        noiseEffects(initializeNoiseEffects(noiseEffects_)) {
        sanityCheckOfNoiseProbabilities(noiseProbability_, amplitudeDampingProb, multiQubitGateFactor_);
    }

    std::vector<std::map<std::string, size_t>> classicalMeasurementsMaps;
    std::map<std::string, size_t>              finalClassicalMeasurementsMap;

    std::map<std::string, std::size_t> simulate(std::size_t shots) override;

    [[nodiscard]] std::size_t getMaxMatrixNodeCount() const override {
        return 0U;
    } // Not available for stochastic simulation
    [[nodiscard]] std::size_t getMatrixActiveNodeCount() const override {
        return 0U;
    } // Not available for stochastic simulation
    [[nodiscard]] std::size_t countNodesFromRoot() override {
        return 0U;
    } // Not available for stochastic simulation

    static void sanityCheckOfNoiseProbabilities(double noiseProbability, double amplitudeDampingProb, double multiQubitGateFactor);

    static std::vector<dd::NoiseOperations> initializeNoiseEffects(const std::string& cNoiseEffects);

    std::map<std::string, std::string> additionalStatistics() override;

private:
    double      noiseProbability{};
    double      amplitudeDampingProb{};
    double      multiQubitGateFactor{};
    std::size_t stochasticRuns{};
    std::size_t maxInstances{};

    std::vector<dd::NoiseOperations> noiseEffects;

    double stochRunTime{};

    void runStochSimulationForId(std::size_t stochRun, qc::Qubit nQubits, std::map<std::string, size_t>& classicalMeasurementsMap, std::uint64_t localSeed);
};
