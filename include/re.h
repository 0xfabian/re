#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <stack>
#include <set>

void expand(std::string& input);

bool is_operator(char c);

int prec(char c);
std::string infix_to_postfix(const std::string& expr);

struct Node
{
    char c;

    Node* parent;
    Node* left;
    Node* right;

    int pos = 0;
    bool nullable = false;
    std::set<int> firstpos;
    std::set<int> lastpos;

    Node(char _c) : c(_c), parent(nullptr), left(nullptr), right(nullptr) {}
};

Node* build_tree(const std::string& input);

void assign_pos(Node* node);
void compute_nullable(Node* node);
void compute_firstpos(Node* node);
void compute_lastpos(Node* node);
void compute_followpos(Node* node, std::vector<std::set<int>>& followpos);

struct DFA
{
    Node* tree;

    std::vector<std::vector<int>> transitions;
    std::set<int> final_states;

    bool match(const std::string& str);
};

bool build_dfa(DFA* dfa, std::string& input);

void print_tree(Node* node, const std::string& prefix = "", bool last_entry = true);
void print_dfa(DFA* dfa);
