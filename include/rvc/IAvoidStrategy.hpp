#pragma once

#include "rvc/Types.hpp"

namespace rvc {

class IAvoidStrategy {
  public:
    virtual ~IAvoidStrategy() = default;

    virtual Direction decideDirection(bool front, bool left, bool right) = 0;
    virtual bool needsReverse(bool front, bool left, bool right) = 0;
};

class DefaultAvoidStrategy final : public IAvoidStrategy {
  public:
    Direction decideDirection(bool front, bool left, bool right) override;
    bool needsReverse(bool front, bool left, bool right) override;
};

} // namespace rvc
