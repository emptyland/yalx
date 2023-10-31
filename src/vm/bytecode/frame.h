#pragma once
#ifndef YALX_VM_BYTECODE_FRAME_H_
#define YALX_VM_BYTECODE_FRAME_H_

#include "base/checking.h"
#include "base/arena-utils.h"
#include "base/base.h"

namespace yalx {

namespace vm {

/*
 * ============-------- caller stack
 * | prev_pc  |
 * +----------+
 * | prev_fp  |
 * +----------+
 * | slots[0] |
 * +----------+
 * | slots[1] |
 * +----------+
 * | ...      |
 * +----------+
 * | slots[n] |
 * +----------+-------
 * |  ret[0]  |      ^
 * +----------+      |
 * |  ret[1]  |      |
 * +----------+   returning vals
 * | ...      |      |
 * +----------+      |
 * |  ret[n]  |      v
 * ============------- callee stack
 * | prev_pc  |
 * +----------+
 * | prev_fp  |
 * +----------+-------
 * | args[0]  |      ^
 * +----------+      |
 * | args[1]  |      |
 * +----------+     args
 * | ...      |      |
 * +----------+      |
 * | args[n]  |      v
 * +----------+-------
 * | slots[0] |      ^
 * +----------+      |
 * | slots[1] |      |
 * +----------+  local vars
 * | ...      |      |
 * +----------+      |
 * | slots[n] |      v
 * +----------+-------
 */

} // namespace vm

} // namespace yalx
