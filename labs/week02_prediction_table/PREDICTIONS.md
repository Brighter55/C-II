# Concept Lab 2 — Prediction Table

**Name:**  Peter Sriphrakhun    **Date:** 2026-10-30

---

## Step 1 — Predict. Do this BEFORE you compile.

| # | The call | Copies | Moves | What the function receives |
|:--|:--|:--:|:--:|:--|
| 1 | `by_value(original)` | 1 | 0 | A copy. the original is untouched. |
| 2 | `by_const_ref(original)` | 0 | 0 | The original itself. |
| 3 | `by_ref(original)` | 0 | 0 | The original itself; `by_ref` appends `!` so it becomes `hero!`. |
| 4 | `by_pointer(&original)` | 0 | 0 | The original through the pointer. |
| 5 | `by_value(make_probe("temp"))` | 0 | 1 | A moved-in temporary named `temp`. |
| 6 | `Holder h{original}` | 1 | 1 | A copy for the sink parameter; `held` then takes it by move. |

**Predicted totals:** copies **2**   moves **2**

> Once you compile, you cannot un-see the answer. Fill the table in first.

---

## Step 2 — Run it.

```bash
cmake --preset course-debug
cmake --build --preset course-debug
./build/course-debug/prediction_table
```

Record the actual numbers:

| # | Actual copies | Actual moves | Matched my prediction? |
|:--|:--:|:--:|:--:|
| 1 | 1 | 0 | Yes |
| 2 | 0 | 0 | Yes |
| 3 | 0 | 0 | Yes |
| 4 | 0 | 0 | Yes |
| 5 | 0 | 0 | **No** |
| 6 | 1 | 1 | Yes |

**Actual totals:** copies **2**   moves **1**

---

## Step 3 — Reconcile. This is the graded artifact.

### Row 5 — `by_value(make_probe("temp"))`

*What I expected:* I expected one move and no copies, because I assumed the temporary created inside `make_probe` had to be moved out of the return value and then moved again into `by_value`'s parameter.

*What actually happened:* No move happened. 

---

## Step 4 — The closing question

**Which one of the six calls would you change, and to what?**

*My answer:* I would change call 1 from `Probe p` to `const Probe& p`, because passing by value deep-copies the `Probe` and a heap-allocated `std::string`on every call, and `by_value` only reads the name. The cost it removes is one string allocation and character copy per call; the cost it adds is that the caller must keep the `Probe` alive for the duration of the call, which is already true here.

---

## What is being graded

| | |
|:--|:--|
| All six predictions recorded **before** the first compile | required |
| Reconciliation written for **every** incorrect row | required |
| The closing recommendation names a cost, not a preference | required |
| The reconciliation write-up | **the graded artifact** — the table alone is not |

Submit this file, completed, as directed on the assignment page.
