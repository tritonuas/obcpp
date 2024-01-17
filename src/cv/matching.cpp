#include "cv/matching.hpp"

Matching::Matching(std::array<CompetitionBottle, NUM_AIRDROP_BOTTLES>
                       competitionObjectives,
                   double matchThreshold)
    : competitionObjectives(competitionObjectives),
      matchThreshold(matchThreshold) {}


MatchResult Matching::match(const CroppedTarget& croppedTarget) {
    return MatchResult{};
}
