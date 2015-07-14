#include <vector>
using namespace std;

typedef struct _grammar_symbol {
	char symbol[32];
} grammar_symbol;

typedef struct _grammar_rule {
	grammar_symbol nonterminal;
	grammar_symbol rule;
} grammar_rule;

typedef struct _regular_grammar {
	vector<grammar_symbol> nonterminals;
	vector<grammar_symbol> terminals;
	vector<grammar_rule> rules;
} regular_grammar;

#define DFA_STATE_INITIAL	1
#define DFA_STATE_FINAL		2

typedef struct _fa_state {
	char label[64];
	int type;
} fa_state;

typedef struct _fa_edge {
	char label[64];
	fa_state * s1;
	fa_state * s2;
} fa_edge;

typedef struct _finite_automaton {
	vector<fa_state> states;
	vector<fa_edge> edges;
} finite_automaton;

void build_dfa(regular_grammar * rg) {
	//vector<grammar_rule>
}