# It's the compile time read-only [Trie](https://en.wikipedia.org/wiki/Trie) implementation.

## Why?

For maximum performance and memory efficiency.
(and for fun even more. I am not sure about the performance yet).


Everything is done in compile time except of searching, of course.

## Usage:

1. Create the Trie from compile-time strings:

        #include "template-tools/trie/trie.hpp"
        #include "template-tools/typestring/typestring.hpp"

        using namespace TypeStringLiteralExploder;

        using Trie = StaticTrie::StaticTrie<
            std::tuple <
                TS("tease"),
                TS("arrest"),
                TS("trail"),
                TS("rhyme"),
                TS("mindless"),
                TS("regret"),
                TS("quack"),
                TS("copy"),
                TS("nosy"),
                TS("doctor"),
                TS("metal"),
                TS("fade"),
                TS("strange"),
                TS("care"),
                TS("bubble"),
                .....
            >
        >
    *Memory pressure during compilation is quite scary, but it looks relatively ok for ~100 strings.*

2. Define the search callback

        auto clb = [&](auto matchInfo) -> Char {
            using MatchInfo = decltype (matchInfo);
            iterateTypeTuple((typename MatchInfo::MatchedStrings*)nullptr, [](auto * s){

                // Access to currently matched string objects:

                using S = typename std::remove_pointer_t<decltype (s)>::ItemT;
                cout << "\t " << S::c_str() << endl;
            });

            // Current node string:

            using S = typename MatchInfo::NodeString

            if constexpr(matchInfo.hasFull) {

                // We are here if there is full string in this Trie node

                cout << "Full Node string: " << S::c_str() << endl;
            }
            if constexpr(matchInfo.isLast) {

                // We are here if this node is last in Trie branch

                cout << "End Node!" << endl;
            }

            // Return the next char for searching if you want to continue or -1 to stop search
            return -1;
        };

3. Start the search by passing the first character

        Trie::search(char0, clb);

4. If you want to use another static-string-like type, look into trie.hpp for requirements.
