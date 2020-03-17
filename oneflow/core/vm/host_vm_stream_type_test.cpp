#define private public
#include "oneflow/core/common/util.h"
#include "oneflow/core/vm/control_vm_stream_type.h"
#include "oneflow/core/vm/host_vm_stream_type.h"
#include "oneflow/core/vm/scheduler.msg.h"
#include "oneflow/core/vm/vm_desc.msg.h"
#include "oneflow/core/common/cached_object_msg_allocator.h"

namespace oneflow {
namespace vm {

namespace test {

namespace {

using InstructionMsgList = OBJECT_MSG_LIST(InstructionMsg, vm_instr_msg_link);

TEST(HostStreamType, basic) {
  auto host_vm_stream_desc = ObjectMsgPtr<StreamDesc>::New(HostStreamType::kStreamTypeId, 1, 1, 1);
  auto vm_desc = ObjectMsgPtr<VmDesc>::New();
  vm_desc->mut_vm_stream_type_id2desc()->Insert(host_vm_stream_desc.Mutable());
  auto scheduler = ObjectMsgPtr<Scheduler>::New(vm_desc.Get());
  InstructionMsgList list;
  uint64_t symbol_value = 9527;
  list.EmplaceBack(ControlStreamType().NewSymbol(symbol_value, 1));
  list.EmplaceBack(HostStreamType().CudaMallocHost(symbol_value, 1024));
  list.EmplaceBack(HostStreamType().CudaFreeHost(symbol_value));
  list.EmplaceBack(ControlStreamType().DeleteSymbol(symbol_value));
  scheduler->Receive(&list);
  scheduler->Schedule();
  scheduler->mut_vm_thread_list()->Begin()->ReceiveAndRun();
  scheduler->Schedule();
  scheduler->mut_vm_thread_list()->Begin()->ReceiveAndRun();
  scheduler->Schedule();
  ASSERT_EQ(scheduler->waiting_vm_instr_chain_list().size(), 0);
  ASSERT_EQ(scheduler->active_vm_stream_list().size(), 0);
  auto* vm_thread = scheduler->mut_vm_thread_list()->Begin();
  ASSERT_TRUE(vm_thread != nullptr);
  auto* vm_stream = vm_thread->mut_vm_stream_list()->Begin();
  ASSERT_TRUE(vm_stream != nullptr);
  auto* vm_instr_chain = vm_stream->mut_running_chain_list()->Begin();
  ASSERT_TRUE(vm_instr_chain == nullptr);
}

TEST(HostStreamType, two_device) {
  int64_t parallel_num = 2;
  auto host_vm_stream_desc =
      ObjectMsgPtr<StreamDesc>::New(HostStreamType::kStreamTypeId, 1, parallel_num, 1);
  auto vm_desc = ObjectMsgPtr<VmDesc>::New();
  vm_desc->mut_vm_stream_type_id2desc()->Insert(host_vm_stream_desc.Mutable());
  auto scheduler = ObjectMsgPtr<Scheduler>::New(vm_desc.Get());
  InstructionMsgList list;
  uint64_t symbol_value = 9527;
  list.EmplaceBack(ControlStreamType().NewSymbol(symbol_value, parallel_num));
  list.EmplaceBack(HostStreamType().CudaMallocHost(symbol_value, 1024));
  scheduler->Receive(&list);
  scheduler->Schedule();
  OBJECT_MSG_LIST_FOR_EACH(scheduler->mut_vm_thread_list(), t) { t->TryReceiveAndRun(); }
}

}  // namespace

}  // namespace test

}  // namespace vm
}  // namespace oneflow
