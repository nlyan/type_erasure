#include <copy_on_write.hpp>

#include "hand_rolled_sbo.hpp"


int main ()
{
#if INSTRUMENT_COPIES
    reset_allocations();
#endif

    std::cout << "sizeof(printable_sbo) = " << sizeof(printable_sbo) << "\n";

#define ECHO(expr)                                                      \
    do {                                                                \
        std::cout << #expr << ";\nap.print() = ";                       \
        expr;                                                           \
        ap.print();                                                     \
        std::cout << "allocations: " << allocations() << "\n\n";        \
        reset_allocations();                                            \
    } while (false)

    ECHO(hi_printable hi; printable_sbo ap(hi));
    ECHO(large_printable large; printable_sbo ap(large));
    ECHO(bye_printable bye; printable_sbo ap(bye));

    ECHO(hi_printable hi; printable_sbo ap = hi);
    ECHO(large_printable large; printable_sbo ap = large);
    ECHO(bye_printable bye; printable_sbo ap = bye);

    ECHO(hi_printable hi; printable_sbo tmp = hi; printable_sbo ap = tmp);
    ECHO(large_printable large; printable_sbo tmp = large; printable_sbo ap = tmp);
    ECHO(bye_printable bye; printable_sbo tmp = bye; printable_sbo ap = tmp);

    ECHO(hi_printable hi; printable_sbo ap; ap = hi);
    ECHO(large_printable large; printable_sbo ap; ap = large);
    ECHO(bye_printable bye; printable_sbo ap; ap = bye);

    ECHO(const hi_printable hi{}; printable_sbo ap(hi));
    ECHO(const large_printable large{}; printable_sbo ap(large));
    ECHO(const bye_printable bye{}; printable_sbo ap(bye));

    ECHO(const hi_printable hi{}; printable_sbo ap = hi);
    ECHO(const large_printable large{}; printable_sbo ap = large);
    ECHO(const bye_printable bye{}; printable_sbo ap = bye);

    ECHO(const hi_printable hi{}; printable_sbo tmp = hi; printable_sbo ap = tmp);
    ECHO(const large_printable large{}; printable_sbo tmp = large; printable_sbo ap = tmp);
    ECHO(const bye_printable bye{}; printable_sbo tmp = bye; printable_sbo ap = tmp);

    ECHO(const hi_printable hi{}; printable_sbo ap; ap = hi);
    ECHO(const large_printable large{}; printable_sbo ap; ap = large);
    ECHO(const bye_printable bye{}; printable_sbo ap; ap = bye);

    ECHO(printable_sbo ap(hi_printable{}));
    ECHO(printable_sbo ap(large_printable{}));
    ECHO(printable_sbo ap(bye_printable{}));

    ECHO(printable_sbo ap = hi_printable{});
    ECHO(printable_sbo ap = large_printable{});
    ECHO(printable_sbo ap = bye_printable{});

    ECHO(hi_printable hi; printable_sbo ap(std::ref(hi)));
    ECHO(large_printable large; printable_sbo ap(std::ref(large)));
    ECHO(bye_printable bye; printable_sbo ap(std::ref(bye)));

    ECHO(hi_printable hi; printable_sbo ap(std::cref(hi)));
    ECHO(large_printable large; printable_sbo ap(std::cref(large)));
    ECHO(bye_printable bye; printable_sbo ap(std::cref(bye)));

#undef ECHO

    {
        std::cout << "copied vector<printable_sbo>{hi_printable, large_printable}" << "\n";

        std::vector<printable_sbo> several_printables = {
            hi_printable{},
            large_printable{}
        };

        for (const auto & printable : several_printables) {
            printable.print();
        }

        std::vector<printable_sbo> several_printables_copy = several_printables;

        std::cout << "allocations: " << allocations() << "\n\n";
        reset_allocations();
    }

    {
        std::cout << "copied vector<COW<printable_sbo>>{hi_printable, large_printable}" << "\n";

        std::vector<copy_on_write<printable_sbo>> several_printables = {
            {hi_printable{}},
            {large_printable{}}
        };

        for (const auto & printable : several_printables) {
            printable->print();
        }

        std::vector<copy_on_write<printable_sbo>> several_printables_copy = several_printables;

        std::cout << "allocations: " << allocations() << "\n\n";
        reset_allocations();
    }

    return 0;
}
