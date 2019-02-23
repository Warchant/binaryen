/*
 * Copyright 2019 WebAssembly Community Group participants
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */


//
// Expand added constants. E.g., consider
//
//  x = y + 10
//  z = x + 20
//  w = load(x + 30)
//
// By expanding out x, we can merge those constants, and this is a pretty common
// pattern. In particular, often big interpreter loops end up having many such
// constant offsets that they care about, and keeping those alive across the
// entire big function is not worthwhile.
//
// Running this after all other optimizations is worthwhile, to see if extra
// added constants like these are optimized away otherhow. In other words, this
// is a good last resort. As such, it will optimize if it made any changes.
//

// XXX now at right speed level on zlib, but size is not great.

#include "wasm.h"
#include "pass.h"
#include "wasm-builder.h"
#include "ir/local-graph.h"

namespace wasm {

// Don't do this on small numbers of locals - the main benefit is when we can remove
// an excessive number of temp locals with long lifetimes.
//////////////////////static const Index MINIMUM_LOCALS = 100;

struct ExpandAddedConstants : public WalkerPass<PostWalker<ExpandAddedConstants>> {
  bool isFunctionParallel() override { return true; }

  Pass* create() override { return new ExpandAddedConstants(optimizing); }

  bool optimizing;

  ExpandAddedConstants(bool optimizing) : optimizing(optimizing) {}

  void doWalkFunction(Function* func) {
  //  if (func->getNumLocals() < MINIMUM_LOCALS) {
 //     return;
//    }
if (getenv("SKIP")) return;
    Builder builder(*getModule());
    LocalGraph localGraph(func);
    // Find which locals have a single set. This is generally what we care about anyhow,
    // as in a big interpreter loop the local will live across the entire function. This
    // also makes it much easier to know that this is safe to do (the parent local is not
    // assigned to in the middle).
    auto safeIndexes = localGraph.getSSAIndexes();
    // Main loop.
    bool changed = false;
    for (auto& pair : localGraph.getSetses) {
      auto* get = pair.first;
      if (safeIndexes.count(get->index)) {
        auto& sets = pair.second;
        if (sets.size() == 1) {
          if (auto* set = *sets.begin()) {
            auto* value = set->value;
            if (auto* binary = value->dynCast<Binary>()) {
              if (binary->op == AddInt32) {
                if (auto* parentGet = binary->left->dynCast<GetLocal>()) {
                  // It's enough to check for a constant on the right, since optimize-instructions
                  // canonicalizes that way.
                  if (auto* c = binary->right->dynCast<Const>()) {
                    // Great, expand it out.
                    *localGraph.locations[get] = builder.makeBinary(
                      AddInt32,
                      builder.makeGetLocal(parentGet->index, parentGet->type),
                      builder.makeConst(c->value)
                    );
                    changed = true;
                  }
                }
              }
            }
          }
        }
      }
    }
    if (optimizing && changed) {
      PassRunner runner(getModule(), getPassOptions());
      runner.setIsNested(true);
      runner.addDefaultFunctionOptimizationPasses();
      runner.runOnFunction(func);
    }
  }
};

Pass *createExpandAddedConstantsPass() {
  return new ExpandAddedConstants(false);
}

Pass *createExpandAddedConstantsOptimizingPass() {
  return new ExpandAddedConstants(true);
}

} // namespace wasm
