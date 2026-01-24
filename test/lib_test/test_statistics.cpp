
#include "../TestMainHelper.h"
#include "core/GameRound.h"
#include "core/Statistics.h"

using namespace evl::core;

namespace {
class StatisticsTest : public testing::Test {
protected:
	void SetUp() override { m_stats = Statistics(); }

	Statistics m_stats;
};
}// namespace

TEST_F(StatisticsTest, DefaultConstructor) {
	EXPECT_EQ(m_stats.lessPickNb, 0);
	EXPECT_TRUE(m_stats.lessPickList.empty());
	EXPECT_EQ(m_stats.mostPickNb, 0);
	EXPECT_TRUE(m_stats.mostPickList.empty());
	EXPECT_EQ(m_stats.roundLessNb, 0);
	EXPECT_EQ(m_stats.roundMostNb, 0);
	EXPECT_DOUBLE_EQ(m_stats.roundAverageNb, 0.0);
	EXPECT_EQ(m_stats.subRoundLessNb, 0);
	EXPECT_EQ(m_stats.subRoundMostNb, 0);
	EXPECT_DOUBLE_EQ(m_stats.subRoundAverageNb, 0.0);
	EXPECT_EQ(m_stats.roundLongest, duration::zero());
	EXPECT_EQ(m_stats.roundShortest, duration::max());
	EXPECT_EQ(m_stats.roundAverage, duration::zero());
	EXPECT_EQ(m_stats.subRoundLongest, duration::zero());
	EXPECT_EQ(m_stats.subRoundShortest, duration::max());
	EXPECT_EQ(m_stats.subRoundAverage, duration::zero());
}

TEST_F(StatisticsTest, CopyConstructor) {
	m_stats.lessPickNb = 5;
	m_stats.mostPickNb = 10;
	const Statistics copy(m_stats);
	EXPECT_EQ(copy.lessPickNb, 5);
	EXPECT_EQ(copy.mostPickNb, 10);
}

TEST_F(StatisticsTest, MoveConstructor) {
	m_stats.lessPickNb = 5;
	m_stats.mostPickNb = 10;
	const Statistics moved(std::move(m_stats));
	EXPECT_EQ(moved.lessPickNb, 5);
	EXPECT_EQ(moved.mostPickNb, 10);
}

TEST_F(StatisticsTest, CopyAssignment) {
	m_stats.lessPickNb = 5;
	const Statistics copy = m_stats;
	EXPECT_EQ(copy.lessPickNb, 5);
}

TEST_F(StatisticsTest, MoveAssignment) {
	m_stats.lessPickNb = 5;
	const Statistics moved = std::move(m_stats);
	EXPECT_EQ(moved.lessPickNb, 5);
}

TEST_F(StatisticsTest, PushRoundNotDone) {
	const GameRound round;
	m_stats.pushRound(round);
	EXPECT_EQ(m_stats.roundLessNb, 0);
	EXPECT_EQ(m_stats.roundMostNb, 0);
	EXPECT_DOUBLE_EQ(m_stats.roundAverageNb, 0.0);
}

TEST_F(StatisticsTest, PushRoundDone) {
	GameRound round;
	round.nextStatus();
	round.addPickedNumber(1);
	round.addPickedNumber(2);
	round.addPickedNumber(3);
	round.addWinner("Test Winner");
	while (round.getStatus() != GameRound::Status::Done) {
		round.addWinner("Test Winner");
		round.nextStatus();
	}

	m_stats.pushRound(round);

	EXPECT_EQ(m_stats.roundLessNb, 3);
	EXPECT_EQ(m_stats.roundMostNb, 3);
	EXPECT_DOUBLE_EQ(m_stats.roundAverageNb, 3.0);
	EXPECT_GT(m_stats.roundLongest, duration::zero());
	EXPECT_GT(m_stats.roundShortest, duration::zero());
	EXPECT_GT(m_stats.roundAverage, duration::zero());
}

TEST_F(StatisticsTest, PushMultipleRounds) {
	GameRound round1;
	round1.nextStatus();
	round1.addPickedNumber(1);
	round1.addPickedNumber(2);
	round1.addWinner("Test Winner");
	while (round1.getStatus() != GameRound::Status::Done) {
		round1.addWinner("Test Winner");
		round1.nextStatus();
	}

	GameRound round2;
	round2.nextStatus();
	round2.addPickedNumber(3);
	round2.addPickedNumber(4);
	round2.addPickedNumber(5);
	round2.addPickedNumber(6);
	round2.addWinner("Test Winner");
	while (round2.getStatus() != GameRound::Status::Done) {
		round2.addWinner("Test Winner");
		round2.nextStatus();
	}

	m_stats.pushRound(round1);
	m_stats.pushRound(round2);

	EXPECT_EQ(m_stats.roundLessNb, 2);
	EXPECT_EQ(m_stats.roundMostNb, 4);
	EXPECT_DOUBLE_EQ(m_stats.roundAverageNb, 3.0);
}

TEST_F(StatisticsTest, PushRoundWithSubRounds) {
	GameRound round;
	round.nextStatus();
	round.addPickedNumber(1);
	round.addPickedNumber(2);
	round.addWinner("Test Winner");
	round.nextStatus();
	round.addPickedNumber(3);
	round.addWinner("Test Winner");
	while (round.getStatus() != GameRound::Status::Done) {
		round.addWinner("Test Winner");
		round.nextStatus();
	}

	m_stats.pushRound(round);

	EXPECT_EQ(m_stats.subRoundLessNb, 0);
	EXPECT_EQ(m_stats.subRoundMostNb, 2);
	EXPECT_GT(m_stats.subRoundLongest, duration::zero());
	EXPECT_GT(m_stats.subRoundShortest, duration::zero());
}

TEST_F(StatisticsTest, PickCountsUpdate) {
	GameRound round;
	round.nextStatus();
	round.addPickedNumber(1);
	round.addPickedNumber(1);
	round.addPickedNumber(2);
	round.addWinner("Test Winner");
	while (round.getStatus() != GameRound::Status::Done) {
		round.addWinner("Test Winner");
		round.nextStatus();
	}

	m_stats.pushRound(round);

	EXPECT_EQ(m_stats.mostPickNb, 2);
	EXPECT_EQ(m_stats.lessPickNb, 0);
	EXPECT_FALSE(m_stats.mostPickList.empty());
	EXPECT_EQ(m_stats.mostPickList[0], 1);
}

TEST_F(StatisticsTest, LessPickStrEmpty) { EXPECT_EQ(m_stats.lessPickStr(), "--"); }

TEST_F(StatisticsTest, LessPickStrWithValues) {
	GameRound round;
	round.nextStatus();
	round.addPickedNumber(1);
	round.addPickedNumber(2);
	round.addWinner("Test Winner");
	while (round.getStatus() != GameRound::Status::Done) {
		round.addWinner("Test Winner");
		round.nextStatus();
	}

	m_stats.pushRound(round);

	const auto str = m_stats.lessPickStr();
	EXPECT_FALSE(str.empty());
	EXPECT_NE(str, "--");
}

TEST_F(StatisticsTest, MostPickStrEmpty) { EXPECT_EQ(m_stats.mostPickStr(), "--"); }

TEST_F(StatisticsTest, MostPickStrWithValues) {
	GameRound round;
	round.nextStatus();
	round.addPickedNumber(1);
	round.addPickedNumber(1);
	round.addPickedNumber(2);
	round.addWinner("Test Winner");
	while (round.getStatus() != GameRound::Status::Done) {
		round.addWinner("Test Winner");
		round.nextStatus();
	}

	m_stats.pushRound(round);

	const auto str = m_stats.mostPickStr();
	EXPECT_FALSE(str.empty());
	EXPECT_NE(str, "--");
	EXPECT_EQ(str, "1");
}

TEST_F(StatisticsTest, MultipleDrawsSameNumber) {
	GameRound round;
	round.nextStatus();
	for (int i = 0; i < 10; ++i) { round.addPickedNumber(5); }
	round.addWinner("Test Winner");
	while (round.getStatus() != GameRound::Status::Done) {
		round.addWinner("Test Winner");
		round.nextStatus();
	}

	m_stats.pushRound(round);
	EXPECT_EQ(m_stats.mostPickNb, 10);
	EXPECT_EQ(m_stats.mostPickList.size(), 1);
	EXPECT_EQ(m_stats.mostPickList[0], 5);
}

TEST_F(StatisticsTest, AllNumbersDrawnOnce) {
	GameRound round;
	round.nextStatus();
	for (uint8_t i = 1; i <= 90; ++i) { round.addPickedNumber(i); }
	round.addWinner("Test Winner");
	while (round.getStatus() != GameRound::Status::Done) {
		round.addWinner("Test Winner");
		round.nextStatus();
	}

	m_stats.pushRound(round);

	EXPECT_EQ(m_stats.mostPickNb, 1);
	EXPECT_EQ(m_stats.lessPickNb, 1);
	EXPECT_EQ(m_stats.mostPickList.size(), 90);
	EXPECT_EQ(m_stats.lessPickList.size(), 90);
}
