# TRIAGE.md

Replace this file with your Build 1 write-up. Three sections, in this order.

## 1. The three defects

For each defect: **how you found it**, **what the actual cause was**, and **what you changed**.

The cause is the part that earns the marks. Name the mechanism, not the line number.

### Defect 1 —
*How I found it:*
*What was actually wrong:*
*What I changed:*

### Defect 2 —
*How I found it:*
*What was actually wrong:*
*What I changed:*

### Defect 3 —
*How I found it:*
*What was actually wrong:*
*What I changed:*

## 2. Ownership table

Every heap allocation in the original, who owned it, and what you converted it to.

| Allocation site | Who owned it before | Converted to | Why that choice |
|:--|:--|:--|:--|
|  |  |  |  |

List your non-owning pointers and references separately, and say how a reader can tell they are non-owning.

## 3. The three worst things about this codebase

With line numbers, and what you would do about each. If something is a later week's
build, say so — knowing what not to fix yet is part of the job.

1.
2.
3.
