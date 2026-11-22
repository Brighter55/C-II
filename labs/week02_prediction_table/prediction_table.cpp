// ===========================================================================
//  IS 267 — Concept Lab 2:  Prediction Table
//
//  DO NOT MODIFY THIS FILE.  It is the thing you are predicting.
//
//  Six calls move a Probe object across four kinds of parameter: by value,
//  by const reference, by reference, and by pointer.  Every construction,
//  copy, move and destruction announces itself.
//
//  Fill in PREDICTIONS.md BEFORE you compile.  That is the whole lab: the
//  value is in being wrong on paper and finding out why, which does not
//  happen if you run it first and write down what you saw.
// ===========================================================================
#include <iostream>
#include <string>
#include <utility>

// ---------------------------------------------------------------- the probe
struct Probe {
    std::string name;
    static int copies;
    static int moves;

    explicit Probe(std::string n) : name(std::move(n)) {
        std::cout << "  [ctor]      " << name << '\n';
    }
    Probe(const Probe& other) : name(other.name + "-copy") {
        ++copies;
        std::cout << "  [COPY ctor] " << other.name << " -> " << name << '\n';
    }
    Probe(Probe&& other) noexcept : name(std::move(other.name)) {
        ++moves;
        other.name = "(moved-from)";
        std::cout << "  [move ctor] " << name << '\n';
    }
    Probe& operator=(const Probe& other) {
        ++copies;
        name = other.name + "-assigned";
        std::cout << "  [COPY assign]\n";
        return *this;
    }
    Probe& operator=(Probe&& other) noexcept {
        ++moves;
        name = std::move(other.name);
        other.name = "(moved-from)";
        std::cout << "  [move assign]\n";
        return *this;
    }
    ~Probe() { std::cout << "  [dtor]      " << name << '\n'; }
};

int Probe::copies = 0;
int Probe::moves  = 0;

// --------------------------------------------------- the six ways to pass it
void by_value(Probe p)              { std::cout << "    inside by_value: " << p.name << '\n'; }
void by_const_ref(const Probe& p)   { std::cout << "    inside by_const_ref: " << p.name << '\n'; }
void by_ref(Probe& p)               { p.name += "!"; std::cout << "    inside by_ref: " << p.name << '\n'; }
void by_pointer(Probe* p)           { std::cout << "    inside by_pointer: " << p->name << '\n'; }

// Returns by value. Worth predicting on its own.
Probe make_probe(const std::string& n) { return Probe{n}; }

// Takes by value and keeps it. The idiomatic "sink" parameter.
struct Holder {
    Probe held;
    explicit Holder(Probe p) : held(std::move(p)) {}
};

static void banner(const char* label) {
    std::cout << "\n--- " << label << " ---\n";
}

static void report(const char* label, int c0, int m0) {
    std::cout << "  => " << label
              << ": copies +" << (Probe::copies - c0)
              << ", moves +"  << (Probe::moves  - m0) << '\n';
}

int main() {
    std::cout << "IS 267 Concept Lab 2 - Prediction Table\n";
    std::cout << "Six calls. Predict first; this output is the answer key.\n";

    int c, m;

    // ---- Call 1 -----------------------------------------------------------
    banner("Call 1: by_value(original)");
    {
        Probe original{"hero"};
        c = Probe::copies; m = Probe::moves;
        by_value(original);
        report("Call 1", c, m);
    }

    // ---- Call 2 -----------------------------------------------------------
    banner("Call 2: by_const_ref(original)");
    {
        Probe original{"hero"};
        c = Probe::copies; m = Probe::moves;
        by_const_ref(original);
        report("Call 2", c, m);
    }

    // ---- Call 3 -----------------------------------------------------------
    banner("Call 3: by_ref(original)");
    {
        Probe original{"hero"};
        c = Probe::copies; m = Probe::moves;
        by_ref(original);
        std::cout << "    after by_ref, original is: " << original.name << '\n';
        report("Call 3", c, m);
    }

    // ---- Call 4 -----------------------------------------------------------
    banner("Call 4: by_pointer(&original)");
    {
        Probe original{"hero"};
        c = Probe::copies; m = Probe::moves;
        by_pointer(&original);
        report("Call 4", c, m);
    }

    // ---- Call 5 -----------------------------------------------------------
    banner("Call 5: by_value(make_probe(\"temp\"))  -- a temporary argument");
    {
        c = Probe::copies; m = Probe::moves;
        by_value(make_probe("temp"));
        report("Call 5", c, m);
    }

    // ---- Call 6 -----------------------------------------------------------
    banner("Call 6: Holder h{original}  -- sink parameter, lvalue argument");
    {
        Probe original{"hero"};
        c = Probe::copies; m = Probe::moves;
        Holder h{original};
        std::cout << "    holder holds: " << h.held.name << '\n';
        report("Call 6", c, m);
    }

    std::cout << "\n=======================================\n";
    std::cout << "TOTAL copies: " << Probe::copies << '\n';
    std::cout << "TOTAL moves : " << Probe::moves  << '\n';
    std::cout << "=======================================\n";
    std::cout << "\nNow open PREDICTIONS.md and reconcile every row you got wrong.\n";
}
