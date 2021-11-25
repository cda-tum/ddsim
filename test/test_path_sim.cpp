#include "PathSimulator.hpp"
#include "algorithms/Entanglement.hpp"
#include "algorithms/Grover.hpp"
#include "dd/Export.hpp"

#include <gtest/gtest.h>
#include <memory>

using namespace dd::literals;

TEST(TaskBasedSimTest, SimpleCircuit) {
    auto qc = std::make_unique<qc::QuantumComputation>(2);
    qc->h(1U);
    qc->x(0U, 1_pc);

    // construct simulator and generate sequential contraction plan
    PathSimulator tbs(std::move(qc), PathSimulator::Configuration());

    // simulate circuit
    auto counts = tbs.Simulate(1024);

    EXPECT_TRUE(tbs.dd->getValueByPath(tbs.root_edge, 0).approximatelyEquals({dd::SQRT2_2, 0}));
    EXPECT_TRUE(tbs.dd->getValueByPath(tbs.root_edge, 3).approximatelyEquals({dd::SQRT2_2, 0}));

    for (const auto& [state, count]: counts) {
        std::cout << state << ": " << count << std::endl;
    }
}

TEST(TaskBasedSimTest, SimpleCircuitAssumeFalseOrder) {
    auto qc = std::make_unique<qc::QuantumComputation>(2);
    qc->h(1U);
    qc->x(0U, 1_pc);
    PathSimulator tbs(std::move(qc), PathSimulator::Configuration());
    // construct simulator and generate sequential contraction plan
    PathSimulator::SimulationPath::ComponentsList path{};
    path.emplace_back(1, 0);
    path.emplace_back(3, 2);
    tbs.setSimulationPath(path, false);
    // simulate circuit
    auto counts = tbs.Simulate(1024);

    EXPECT_TRUE(tbs.dd->getValueByPath(tbs.root_edge, 0).approximatelyEquals({dd::SQRT2_2, 0}));
    EXPECT_TRUE(tbs.dd->getValueByPath(tbs.root_edge, 3).approximatelyEquals({dd::SQRT2_2, 0}));

    for (const auto& [state, count]: counts) {
        std::cout << state << ": " << count << std::endl;
    }
}

TEST(TaskBasedSimTest, GroverCircuitBracket) {
    std::unique_ptr<qc::QuantumComputation> qc          = std::make_unique<qc::Grover>(4, 12345);
    auto                                    grover      = dynamic_cast<qc::Grover*>(qc.get());
    auto                                    targetValue = grover->targetValue;

    // construct simulator and generate sequential contraction plan
    PathSimulator tbs(std::move(qc), PathSimulator::Configuration(PathSimulator::Configuration::Mode::BracketGrouping, 3));

    // simulate circuit
    auto counts = tbs.Simulate(4096);

    auto c    = tbs.dd->getValueByPath(tbs.root_edge, targetValue);
    auto prob = c.r * c.r + c.i * c.i;
    EXPECT_GT(prob, 0.9);

    dd::export2Dot(tbs.root_edge, "result_grover.dot", true, true);

    for (const auto& [state, count]: counts) {
        std::cout << state << ": " << count << std::endl;
    }
}

TEST(TaskBasedSimTest, GroverCircuitAlternatingMiddle) {
    std::unique_ptr<qc::QuantumComputation> qc          = std::make_unique<qc::Grover>(4, 12345);
    auto                                    grover      = dynamic_cast<qc::Grover*>(qc.get());
    auto                                    targetValue = grover->targetValue;

    // construct simulator and generate sequential contraction plan
    PathSimulator tbs(std::move(qc), PathSimulator::Configuration(PathSimulator::Configuration::Mode::Alternating, 0));

    // simulate circuit
    auto counts = tbs.Simulate(4096);

    auto c    = tbs.dd->getValueByPath(tbs.root_edge, targetValue);
    auto prob = c.r * c.r + c.i * c.i;
    EXPECT_GT(prob, 0.9);

    dd::export2Dot(tbs.root_edge, "result_grover.dot", true, true);

    for (const auto& [state, count]: counts) {
        std::cout << state << ": " << count << std::endl;
    }
}

TEST(TaskBasedSimTest, GroverCircuitAlternatingRandom) {
    std::unique_ptr<qc::QuantumComputation> qc          = std::make_unique<qc::Grover>(4, 12345);
    auto                                    grover      = dynamic_cast<qc::Grover*>(qc.get());
    auto                                    targetValue = grover->targetValue;

    // construct simulator and generate sequential contraction plan
    PathSimulator tbs(std::move(qc), PathSimulator::Configuration(PathSimulator::Configuration::Mode::Alternating, 6));

    // simulate circuit
    auto counts = tbs.Simulate(4096);

    auto c    = tbs.dd->getValueByPath(tbs.root_edge, targetValue);
    auto prob = c.r * c.r + c.i * c.i;
    EXPECT_GT(prob, 0.9);

    dd::export2Dot(tbs.root_edge, "result_grover.dot", true, true);

    for (const auto& [state, count]: counts) {
        std::cout << state << ": " << count << std::endl;
    }
}

TEST(TaskBasedSimTest, SimpleCircuitBracket) {
    auto qc = std::make_unique<qc::QuantumComputation>(2);
    qc->h(1U);
    qc->x(0U, 1_pc);
    qc->x(0U, 1_pc);
    qc->x(0U, 1_pc);

    // construct simulator and generate sequential contraction plan
    PathSimulator tbs(std::move(qc), PathSimulator::Configuration(PathSimulator::Configuration::Mode::BracketGrouping, 3));

    // simulate circuit
    auto counts = tbs.Simulate(1024);

    for (const auto& [state, count]: counts) {
        std::cout << state << ": " << count << std::endl;
    }
}

TEST(TaskBasedSimTest, GroverCircuitPairwiseGrouping) {
    std::unique_ptr<qc::QuantumComputation> qc          = std::make_unique<qc::Grover>(4, 12345);
    auto                                    grover      = dynamic_cast<qc::Grover*>(qc.get());
    auto                                    targetValue = grover->targetValue;
    grover->print(std::cout);

    // construct simulator and generate sequential contraction plan
    PathSimulator tbs(std::move(qc), PathSimulator::Configuration(PathSimulator::Configuration::Mode::PairwiseRecursiveGrouping, 0));

    // simulate circuit
    auto counts = tbs.Simulate(4096);

    auto c    = tbs.dd->getValueByPath(tbs.root_edge, targetValue);
    auto prob = c.r * c.r + c.i * c.i;
    EXPECT_GT(prob, 0.9);

    dd::export2Dot(tbs.root_edge, "result_grouping.dot", true, true);

    for (const auto& [state, count]: counts) {
        std::cout << state << ": " << count << std::endl;
    }
}
