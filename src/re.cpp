#include "re.h"

void expand(std::string& input)
{
    std::string ret = "";

    bool in_brackets = false;

    for (int i = 0; i < input.size(); i++)
    {
        if (input[i] == '[')
            in_brackets = true;
        else if (input[i] == ']')
            in_brackets = false;

        if (in_brackets && input[i] == '-' && (i + 1) < input.size())
        {
            char first = input[i - 1];
            char second = input[i + 1];

            for (char c = first + 1; c < second; c++)
                ret += c;
        }
        else
            ret += input[i];
    }

    input = ret;
    ret.clear();

    in_brackets = false;

    for (int i = 0; i < input.size(); i++)
    {
        if (input[i] == '[')
        {
            in_brackets = true;
            ret += '(';
        }
        else if (input[i] == ']')
        {
            in_brackets = false;
            ret += ')';
        }
        else
        {
            if (in_brackets && input[i - 1] != '[')
                ret += '|';

            ret += input[i];
        }
    }

    input = ret;
    ret.clear();

    for (int i = 0; i < input.size(); i++)
    {
        ret += input[i];

        if (input[i] != '(' && input[i] != '|' && (i + 1) < input.size())
        {
            if (input[i + 1] != '*' && input[i + 1] != '|' && input[i + 1] != ')')
                ret += '.';
        }
    }

    input = ret;
}

bool is_operator(char c)
{
    return c == '.' || c == '|' || c == '*';
}

int prec(char c)
{
    switch (c)
    {
    case '*':   return 3;
    case '.':   return 2;
    case '|':   return 1;
    default:    return -1;
    }
}

std::string infix_to_postfix(const std::string& expr)
{
    std::string res = "";
    std::stack<char> st;

    for (char c : expr)
    {
        if (is_operator(c))
        {
            while (!st.empty() && prec(c) <= prec(st.top()))
            {
                res += st.top();
                st.pop();
            }

            st.push(c);
        }
        else if (c == '(')
        {
            st.push('(');
        }
        else if (c == ')')
        {
            while (st.top() != '(')
            {
                res += st.top();
                st.pop();
            }

            st.pop();
        }
        else
            res += c;
    }

    while (!st.empty())
    {
        res += st.top();
        st.pop();
    }

    return res;
}

Node* build_tree(const std::string& input)
{
    if (input.empty())
        return nullptr;

    int i = input.size() - 1;

    Node* root = new Node(input[i--]);
    Node* node = is_operator(root->c) ? root : nullptr;

    while (i >= 0)
    {
        if (!node)
            return nullptr;

        Node* child = new Node(input[i--]);
        child->parent = node;

        if (node->c == '.' || node->c == '|')
        {
            if (!node->right)
                node->right = child;
            else
                node->left = child;
        }
        else if (node->c == '*')
            node->right = child;

        if (is_operator(child->c))
            node = child;
        else
        {
            while (node)
            {
                if ((node->c == '.' || node->c == '|') && !node->left)
                    break;

                node = node->parent;
            }
        }
    }

    if (!node)
        return root;

    return nullptr;
}

int current_pos = 0;
std::vector<Node*> leafs;

void assign_pos(Node* node)
{
    if (node->right == nullptr && node->left == nullptr)
    {
        leafs.push_back(node);
        node->pos = current_pos;
        current_pos++;
    }
    else
    {
        if (node->right != nullptr)
            assign_pos(node->right);

        if (node->left != nullptr)
            assign_pos(node->left);
    }
}

void compute_nullable(Node* node)
{
    if (node == nullptr)
        return;

    compute_nullable(node->left);
    compute_nullable(node->right);

    if (node->right == nullptr && node->left == nullptr)
        node->nullable = false;
    else if (node->c == '|')
        node->nullable = node->left->nullable || node->right->nullable;
    else if (node->c == '.')
        node->nullable = node->left->nullable && node->right->nullable;
    else if (node->c == '*')
        node->nullable = true;
}

void compute_firstpos(Node* node)
{
    if (node == nullptr)
        return;

    compute_firstpos(node->left);
    compute_firstpos(node->right);

    if (node->right == nullptr && node->left == nullptr)
        node->firstpos.insert(node->pos);
    else if (node->c == '|')
    {
        for (auto& e : node->left->firstpos)
            node->firstpos.insert(e);

        for (auto& e : node->right->firstpos)
            node->firstpos.insert(e);
    }
    else if (node->c == '.')
    {
        for (auto& e : node->left->firstpos)
            node->firstpos.insert(e);

        if (node->left->nullable)
        {
            for (auto& e : node->right->firstpos)
                node->firstpos.insert(e);
        }
    }
    else if (node->c == '*')
    {
        for (auto& e : node->right->firstpos)
            node->firstpos.insert(e);
    }
}

void compute_lastpos(Node* node)
{
    if (node == nullptr)
        return;

    compute_lastpos(node->left);
    compute_lastpos(node->right);

    if (node->right == nullptr && node->left == nullptr)
        node->lastpos.insert(node->pos);
    else if (node->c == '|')
    {
        for (auto& e : node->left->lastpos)
            node->lastpos.insert(e);

        for (auto& e : node->right->lastpos)
            node->lastpos.insert(e);
    }
    else if (node->c == '.')
    {
        for (auto& e : node->right->lastpos)
            node->lastpos.insert(e);

        if (node->right->nullable)
        {
            for (auto& e : node->left->lastpos)
                node->lastpos.insert(e);
        }
    }
    else if (node->c == '*')
    {
        for (auto& e : node->right->lastpos)
            node->lastpos.insert(e);
    }
}

void compute_followpos(Node* node, std::vector<std::set<int>>& followpos)
{
    if (node == nullptr)
        return;

    compute_followpos(node->left, followpos);
    compute_followpos(node->right, followpos);

    for (int i = 0; i < leafs.size(); i++)
    {
        if (node->c == '.')
        {
            if (node->left->lastpos.find(i) != node->left->lastpos.end())
                for (auto& e : node->right->firstpos)
                    followpos[i].insert(e);
        }
        else if (node->c == '*')
        {
            if (node->lastpos.find(i) != node->lastpos.end())
                for (auto& e : node->firstpos)
                    followpos[i].insert(e);
        }
    }
}

struct State
{
    int id;
    std::set<int> data;
};

bool build_dfa(DFA* dfa, std::string& input)
{
    expand(input);

    std::string postfix_input = infix_to_postfix(input) + "\003.";

    dfa->tree = build_tree(postfix_input);

    if (!dfa->tree)
        return false;

    assign_pos(dfa->tree);
    compute_nullable(dfa->tree);
    compute_firstpos(dfa->tree);
    compute_lastpos(dfa->tree);

    std::vector<std::set<int>> followpos(leafs.size());
    compute_followpos(dfa->tree, followpos);

    std::vector<State> open;
    std::vector<State> closed;

    int id = 0;
    dfa->transitions.push_back(std::vector<int>());
    open.push_back((State) { id, dfa->tree->firstpos });

    if (dfa->tree->firstpos.find(0) != dfa->tree->firstpos.end())
        dfa->final_states.insert(id);

    while (!open.empty())
    {
        State state = open.front();
        open.erase(open.begin());
        closed.push_back(state);

        for (int c = 0; c < 256; c++)
        {
            std::set<int> new_state;

            for (auto& e : state.data)
            {
                if (c == leafs[e]->c)
                    for (auto& new_e : followpos[e])
                        new_state.insert(new_e);
            }

            if (new_state.empty())
                dfa->transitions[state.id].push_back(-1);
            else
            {
                bool found = false;

                for (auto& open_state : open)
                    if (open_state.data == new_state)
                    {
                        dfa->transitions[state.id].push_back(open_state.id);
                        found = true;
                        break;
                    }

                if (!found)
                {
                    for (auto& closed_state : closed)
                        if (closed_state.data == new_state)
                        {
                            dfa->transitions[state.id].push_back(closed_state.id);
                            found = true;
                            break;
                        }

                    if (!found)
                    {
                        id++;
                        dfa->transitions.push_back(std::vector<int>());
                        open.push_back((State) { id, new_state });
                        dfa->transitions[state.id].push_back(id);

                        if (new_state.find(0) != new_state.end())
                            dfa->final_states.insert(id);
                    }
                }
            }
        }
    }

    return true;
}

bool DFA::match(const std::string& str)
{
    int state = 0;

    for (auto& c : str)
    {
        if (state == -1)
            break;

        int new_state = transitions[state][c];

        state = new_state;
    }

    if (state == -1)
        return false;

    return final_states.find(state) != final_states.end();
}

void print_tree(Node* node, const std::string& prefix, bool last_entry)
{
    printf("\e[33m%s", prefix.c_str());

    if (is_operator(node->c))
        printf("\e[94m%c\e[0m\n", node->c);
    else
    {
        printf("\e[0m%c\n", node->c);
        return;
    }

    std::string new_prefix = prefix;

    if (new_prefix.size() >= 3)
    {
        new_prefix.erase(new_prefix.size() - 3, 3);
        new_prefix += last_entry ? "   " : "|  ";
    }

    if (node->right)
        print_tree(node->right, new_prefix + (node->left ? "|- " : "`- "), !node->left);

    if (node->left)
        print_tree(node->left, new_prefix + "`- ", true);
}

void print_dfa(DFA* dfa)
{
    printf("  ");

    for (int c = 0; c < 256; c++)
    {
        if (!(c >= '0' && c <= '9') && !(c >= 'a' && c <= 'z'))
            continue;

        printf("%c ", c);
    }

    printf("\n");

    for (int i = 0; i < dfa->transitions.size(); i++)
    {
        if (dfa->final_states.find(i) != dfa->final_states.end())
            printf("\e[92m%d\e[0m ", i);
        else
            printf("%d ", i);

        for (int c = 0; c < 256; c++)
        {
            if (!(c >= '0' && c <= '9') && !(c >= 'a' && c <= 'z'))
                continue;

            int j = dfa->transitions[i][c];

            if (j == -1)
                printf("\e[90m_ \e[0m");
            else
            {
                if (dfa->final_states.find(j) != dfa->final_states.end())
                    printf("\e[92m%d\e[0m ", j);
                else
                    printf("%d ", j);
            }
        }

        printf("\n");
    }
}
