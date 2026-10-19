/* ===========================================================================
   ownership_audit.cpp  --  Concept Lab 1

   A small program with three memory defects in it:

       one leak
       one double delete
       one dangling reference

   Your job is NOT to rewrite this program. Your job is, in order:

     1. Build it with the course-debug preset and run it. Capture what the
        sanitizer says about each defect.
     2. For each defect, write two sentences in ANALYSIS.md: what the
        sanitizer said, and what was actually wrong. Those are different
        things, and the second one is what earns the marks.
     3. Convert the owning allocations to smart pointers so all three
        defects are gone.
     4. Leave non-owning observation as a reference or a raw pointer, and
        mark each one with a comment saying it is non-owning.

   The sanitizer stops at the FIRST problem it finds, so you will not see all
   three in one run. That is normal and it is how real debugging goes: read
   the report, fix that one, rebuild, run again, find the next. Expect three
   rounds. The leak is reported at exit, so it surfaces last -- once the
   program can actually reach the end.

   Read the compiler warnings too. One of these three defects is sitting in
   a warning before you ever run the program.

   expected_output.txt next to this file is what the program should print once
   all three defects are fixed. Diff against it when you are done.

   Build and run:
       cmake --preset course-debug
       cmake --build --preset course-debug
       ./build/course-debug/ownership_audit
   =========================================================================== */

#include <cstdio>
#include <cstring>
#include <string>
#include <vector>

struct Rune {
    char  name[24];
    int   charge;
};

Rune* make_rune(const char* name, int charge)
{
    Rune* r = new Rune;
    std::strncpy(r->name, name, 23);
    r->name[23] = '\0';
    r->charge = charge;
    return r;
}

/* Returns the total charge of every rune in the satchel. */
int total_charge(const std::vector<Rune*>& satchel)
{
    int total = 0;
    for (size_t i = 0; i < satchel.size(); i++)
        total += satchel[i]->charge;
    return total;
}

/* Spends a rune's charge and reports what it was worth. */
int spend_rune(Rune* r)
{
    int spent = r->charge;
    r->charge = 0;
    return spent;
}

/* Builds the starting satchel. */
std::vector<Rune*> build_satchel()
{
    std::vector<Rune*> satchel;
    satchel.push_back(make_rune("Ember",  12));
    satchel.push_back(make_rune("Frost",   8));
    satchel.push_back(make_rune("Gale",   15));

    Rune* spare = make_rune("Spare", 99);
    (void)spare;

    return satchel;
}

/* Finds the strongest rune and hands back a reference to it. */
const Rune& strongest_rune(const std::vector<Rune*>& satchel)
{
    size_t best = 0;
    for (size_t i = 1; i < satchel.size(); i++) {
        if (satchel[i]->charge > satchel[best]->charge)
            best = i;
    }

    Rune snapshot = *satchel[best];
    return snapshot;
}

int main()
{
    std::vector<Rune*> satchel = build_satchel();

    std::printf("runes in satchel ... %d\n", (int)satchel.size());
    std::printf("total charge ....... %d\n", total_charge(satchel));

    const Rune& strongest = strongest_rune(satchel);
    std::printf("strongest rune ..... %s (%d)\n", strongest.name, strongest.charge);

    std::printf("spent Ember ........ %d\n", spend_rune(satchel[0]));
    std::printf("total charge ....... %d\n", total_charge(satchel));

    Rune* gale = satchel[2];
    delete gale;

    for (size_t i = 0; i < satchel.size(); i++)
        delete satchel[i];

    std::printf("satchel emptied.\n");
    return 0;
}
