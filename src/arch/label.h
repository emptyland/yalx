// Copyright 2017 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef YALX_ARCH_LABEL_H_
#define YALX_ARCH_LABEL_H_

#include "base/checking.h"
#include "base/base.h"

namespace yalx {

namespace arm64 {class Assembler;}
namespace x64 {class Assembler;}

namespace arch {

// -----------------------------------------------------------------------------
// Labels represent pc locations; they are typically jump or call targets.
// After declaration, a label can be freely used to denote known or (yet)
// unknown pc location. Assembler::bind() is used to bind a label to the
// current pc. A label can be bound only once.

class Label {
public:
    enum Distance {
        kNear,  // near jump: 8 bit displacement (signed)
        kFar    // far jump: 32 bit displacement (signed)
    };

    Label() = default;

    // Disallow copy construction and assignment, but allow move construction and
    // move assignment on selected platforms (see below).
    Label(const Label&) = delete;
    Label& operator=(const Label&) = delete;

    // On ARM64, the Assembler keeps track of pointers to Labels to resolve
    // branches to distant targets. Copying labels would confuse the Assembler.
    // On other platforms, allow move construction.
#if !YALX_ARCH_ARM64
    // In debug builds, the old Label has to be cleared in order to avoid a DCHECK
    // failure in it's destructor.
#ifndef NDEBUG // DEBUG
    Label(Label&& other) { *this = std::move(other); }
    Label& operator=(Label&& other) {
        pos_ = other.pos_;
        near_link_pos_ = other.near_link_pos_;
        other.Unuse();
        other.UnuseNear();
        return *this;
    }
#else
    Label(Label&&) = default;
    Label& operator=(Label&&) = default;
#endif
#endif

#ifndef NDEBUG // DEBUG
  ~Label() {
      assert(!is_linked());
      assert(!is_near_linked());
  }
#endif

    void Unuse() { pos_ = 0; }
    void UnuseNear() { near_link_pos_ = 0; }

    bool is_bound() const { return pos_ < 0; }
    bool is_unused() const { return pos_ == 0 && near_link_pos_ == 0; }
    bool is_linked() const { return pos_ > 0; }
    bool is_near_linked() const { return near_link_pos_ > 0; }

    // Returns the position of bound or linked labels. Cannot be used
    // for unused labels.
    int pos() const {
        if (pos_ < 0) return -pos_ - 1;
        if (pos_ > 0) return pos_ - 1;
        UNREACHABLE();
    }

    int near_link_pos() const { return near_link_pos_ - 1; }

    friend class arm64::Assembler;
    friend class x64::Assembler;
private:
    void bind_to(int pos) {
        pos_ = -pos - 1;
        assert(is_bound());
    }
    
    void link_to(int pos, Distance distance = kFar) {
        if (distance == kNear) {
            near_link_pos_ = pos + 1;
            assert(is_near_linked());
        } else {
            pos_ = pos + 1;
            assert(is_linked());
        }
    }

    // pos_ encodes both the binding state (via its sign)
    // and the binding position (via its value) of a label.
    //
    // pos_ <  0  bound label, pos() returns the jump target position
    // pos_ == 0  unused label
    // pos_ >  0  linked label, pos() returns the last reference position
    int pos_ = 0;

    // Behaves like |pos_| in the "> 0" case, but for near jumps to this label.
    int near_link_pos_ = 0;
};

} // namespace arch

} // namespace yalx

#endif // YALX_ARCH_LABEL_H_
