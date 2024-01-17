#include <string>
#include <vector>

#include "re.h"

using namespace std;

int main(int argc, char** argv)
{
    bool a_flag = false;
    bool t_flag = false;

    if (argc != 2 && argc != 3)
    {
        fprintf(stderr, "usage: %s expression\n", argv[0]);
        return 1;
    }

    string expr = argv[1];

    if (argc == 3)
    {
        string flag = argv[2];

        if (flag == "-t")
            t_flag = true;
        else if (flag == "-a")
            a_flag = true;
    }

    DFA dfa;

    if (!build_dfa(&dfa, expr))
    {
        fprintf(stderr, "%s: bad expression\n", argv[0]);
        return 1;
    }

    if (t_flag)
        print_tree(dfa.tree->left);
    else if (a_flag)
        print_dfa(&dfa);
    else
    {
        while (true)
        {
            string in;
            getline(cin, in);

            if (!cin)
                break;

            if (dfa.match(in))
                printf("\e[92mtrue\e[0m\n");
            else
                printf("\e[91mfalse\e[0m\n");
        }
    }

    return 0;
}
