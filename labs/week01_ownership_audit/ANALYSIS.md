
## Defect 1 — dangling reference

The compiler caught this one before the program ever ran:

```
ownership_audit.cpp:100:12: warning: reference to local variable 'snapshot'
returned [-Wreturn-local-addr]
  100 |     return snapshot;
      |            ^~~~~~~~
ownership_audit.cpp:99:10: note: declared here
   99 |     Rune snapshot = *satchel[best];
```

Under the sanitizer the first run also stopped here:

```
ownership_audit.cpp:100:12: runtime error: reference binding to null pointer of type 'const struct Rune'
ownership_audit.cpp:111:77: runtime error: member access within null pointer of type 'const struct Rune'
==1==ERROR: AddressSanitizer: SEGV on unknown address 0x000000000018
SUMMARY: AddressSanitizer: SEGV ... in main
```

What the sanitizer said: the reference `strongest_rune` handed back named an object that was no longer alive, and the first read through it faulted. What was actually wrong: the function copied the winning rune into a local `Rune snapshot` and returned a reference to that local, so the reference outlived the stack frame that held the object it named.

## Defect 2 — double delete

Once the dangling reference was fixed, the next run reached this:

```
==1==ERROR: AddressSanitizer: attempting double-free on 0x6030000000a0 in thread T0
    #1 0x402115 in main (the loop's delete)
freed by thread T0 here:
    #1 0x40209a in main (the earlier delete of satchel[2])
previously allocated by thread T0 here:
    #1 0x4012ff in make_rune(char const*, int)
    #2 0x401939 in build_satchel()
SUMMARY: AddressSanitizer: double-free
```

What the sanitizer said: the same 28-byte region was freed twice, and the second free aborted the process. What was actually wrong: `main` deleted the third rune through a separate raw pointer and then ran a cleanup loop that deleted every element of the satchel, so the vector still held an address whose object had already been destroyed and the loop freed it again.

## Defect 3 — leak

With the first two defects fixed, the program ran to completion and LeakSanitizer reported this at exit:

```
==1==ERROR: LeakSanitizer: detected memory leaks

Direct leak of 28 byte(s) in 1 object(s) allocated from:
    #0 operator new(unsigned long)
    #1 make_rune(char const*, int)
    #2 build_satchel()
    #3 main
SUMMARY: AddressSanitizer: 28 byte(s) leaked in 1 allocation(s).
```

What the sanitizer said: one allocation made in `build_satchel` was still owned by nothing at exit, so it was reported as a direct leak. What was actually wrong: `build_satchel` allocated a fourth rune, "Spare", saved the only pointer to it in a local, cast the pointer to `void`, and returned without that pointer ever escaping or being deleted, so no owner existed to free it.