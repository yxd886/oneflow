#ifndef ONEFLOW_CORE_COMMON_RANGE_H_
#define ONEFLOW_CORE_COMMON_RANGE_H_

#include "oneflow/core/common/util.h"
#include "oneflow/core/common/range.pb.h"

namespace oneflow {

class Range final {
 public:
  // OF_DISALLOW_COPY_AND_MOVE(Range);
  Range() : Range(0, 0) {}
  ~Range() = default;

  Range(int64_t begin, int64_t end) : begin_(begin), end_(end) {}
  explicit Range(const RangeProto& range_proto);

  bool operator==(const Range& rhs) const { return begin_ == rhs.begin_ && end_ == rhs.end_; }
  bool operator!=(const Range& rhs) const { return !(*this == rhs); }

  int64_t begin() const { return begin_; }
  int64_t end() const { return end_; }

  int64_t& mut_begin() { return begin_; }
  int64_t& mut_end() { return end_; }

  int64_t size() const { return end_ - begin_; }


  void ToProto(RangeProto* ret) const;

 private:
  int64_t begin_;
  int64_t end_;
};

Range FindIntersectant(const Range& lhs, const Range& rhs);

}  // namespace oneflow

namespace std {

template<>
struct hash<oneflow::Range> {
  size_t operator()(const oneflow::Range& range) const {
    return (range.begin() << 32) ^ range.end();
  }
};

}  // namespace std

#endif  // ONEFLOW_CORE_COMMON_RANGE_H_
