#pragma once


#include <functional>
#include <iostream>
#include <fstream>
#include <string>
#include <stack>
#include <queue>
#include <cmath>
#include <map>

using namespace std;

static map<string, float> variables_;
vector<float> returned;

class ast
{
public:

	ast(const string& file)
	{
		this->file_ = file;

		build_tree();
	}

	void calculate() const
	{
		root_->calculate();

		for (auto& result : returned)
		{
			cout << result << endl;
		}
	}

private:

	string file_;

	struct node
	{
		virtual float calculate()
		{
			return 0;
		}
	};

	class math_tree
	{
	public:

		math_tree() = default;

		explicit math_tree(const string& expression) : root_(nullptr)
		{
			define_operators();

			auto tokens = split_input(expression);

			sorting_yard(tokens);
		}

		float calculate()
		{
			return  calculate(root_);
		}

	private:

		enum token_type {
			num, op, func, par, unary
		};

		struct token
		{
			string sign; token_type type = num;

			token() = default;

			token(const string& sign, const token_type type)
			{
				this->type = type; this->sign = sign;
			}
		};

		struct node
		{
			token value; node* left; node* right;

			node() : left(nullptr), right(nullptr) {}

			explicit node(const token& value, node* left = nullptr,
				node* right = nullptr) : left(left), right(right)
			{
				this->value = value;
			}

		};

		map<string, pair<token_type, int> > operators_;

		map<string, function<float(float&, float&)> > binary_operations_;

		map<string, function<float(float&)> > unary_operations_;

		node* root_;

		float calculate(node* current)
		{
			if (current->value.type == num)
				return stof(current->value.sign);

			if (current->value.type == op)
			{
				auto left = calculate(current->left);

				auto right = calculate(current->right);

				return binary_operations_[current->value.sign](left, right);
			}

			if (current->value.type == unary || current->value.type == func)
			{
				auto left = calculate(current->left);

				return unary_operations_[current->value.sign](left);
			}

			throw runtime_error("Invalid sign");
		}

		void define_operators()
		{
			operators_["+"] = { op, 1 };

			operators_["-"] = { op, 1 };

			operators_["/"] = { op, 2 };

			operators_["*"] = { op, 2 };

			operators_["=="] = { op, 0 };

			operators_[">"] = { op, 0 };

			operators_["<"] = { op, 0 };

			operators_["^"] = { op, 3 };

			operators_["++"] = { unary, 1 };

			operators_["sin"] = { func, 1 };

			operators_["ctg"] = { func, 1 };

			binary_operations_["+"] = [](float& lhs, float& rhs) {
				return lhs + rhs;
			};

			binary_operations_["-"] = [](float& lhs, float& rhs) {
				return lhs - rhs;
			};

			binary_operations_["/"] = [](float& lhs, float& rhs) {
				return lhs / rhs;
			};

			binary_operations_["*"] = [](float& lhs, float& rhs) {
				return lhs * rhs;
			};

			binary_operations_["=="] = [](float& lhs, float& rhs) {
				return lhs == rhs ? 1.0f : 0.0f;
			};
			binary_operations_[">"] = [](float& lhs, float& rhs) {
				return lhs > rhs ? 1.0f : 0.0f;
			};
			binary_operations_["<"] = [](float& lhs, float& rhs) {
				return lhs < rhs ? 1.0f : 0.0f;
			};

			binary_operations_["^"] = [](float& lhs, float& rhs) {
				return pow(lhs, rhs);
			};

			unary_operations_["++"] = [](float& lhs) {
				return lhs + 1;
			};

			unary_operations_["sin"] = [](float& lhs) {
				return sin(lhs);
			};

			unary_operations_["ctg"] = [](float& lhs) {
				return 1.0f / tan(lhs);
			};
		}

		void sorting_yard(vector<token>& tokens)
		{
			stack<node*> out_stack;
			stack<token> op_stack;

			auto add_binary = [](stack<node*>& out_stack,
				stack<token>& op_stack) mutable
			{ 
				auto* right = out_stack.top();
				out_stack.pop();

				auto* left = out_stack.top();
				out_stack.pop();

				out_stack.push(new node(op_stack.top(),
					left, right));
			};

			auto add_unary = [](stack<node*>& out_stack,
				const token& token) mutable
			{
				auto* child = out_stack.top();
				out_stack.pop();

				out_stack.push(new node(token, child));
			};

			for (auto& token : tokens)
			{
				if (token.type == num) {
					out_stack.push(new node(token));
				}

				if (token.type == unary)
				{
					add_unary(out_stack, token);
				}

				if (token.type == func)
					op_stack.push(token);

				if (token.type == op)
				{
					while (!op_stack.empty() && op_stack.top().type == op &&
						operators_[op_stack.top().sign].second >= operators_[token.sign].second)
					{
						add_binary(out_stack, op_stack);

						op_stack.pop();
					}
					op_stack.push(token);
				}

				if (token.sign == "(")
					op_stack.push(token);

				if (token.sign == ")")
				{
					while (op_stack.top().sign != "(")
					{
						add_binary(out_stack, op_stack);

						op_stack.pop();
					}

					op_stack.pop();

					if (!op_stack.empty() && op_stack.top().type == func)
					{
						add_unary(out_stack, op_stack.top());

						op_stack.pop();
					}
				}
			}

			while (!op_stack.empty())
			{
				add_binary(out_stack, op_stack);

				op_stack.pop();
			}

			root_ = out_stack.top();
		}

		vector<token> split_input(const string& expression)
		{
			auto input = expression;

			vector<token> tokens;
			tokens.reserve(input.length());

			unsigned i = 0;

			while (i < input.length())
			{
				while ( i < input.length() && isspace(input[i]))
					i++;
				if (i == input.length())
					break;

				if (input[i] == '(') {
					tokens.emplace_back(token("(", par));
					++i; continue;
				}

				if (input[i] == ')') {
					tokens.emplace_back(token(")", par));
					++i; continue;
				}

				if (isdigit(input[i]))
				{
					const auto start = int(i);

					while (isdigit(input[i]) || input[i] == '.')
						i++;

					const auto length = int(i) - start;

					tokens.emplace_back(token(input.substr(
						start, length), num));
				}

				else {

					string found_var;

					for (auto& variable : variables_)
					{
						if (i + variable.first.length() - 1 < input.length())
						{
							if (variable.first == input.substr(i, variable.first.length()))
							{
								found_var = variable.first.length() > found_var.length() ?
									variable.first : found_var;
							}
						}
					}

					if (!found_var.empty())
					{
						auto sign = to_string(variables_[found_var]);

						tokens.emplace_back(token(sign, num));

						i += found_var.length(); continue;
					}

					pair<string, pair<token_type, int>> found_op;

					for (auto& operator_ : operators_)
					{
						if (i + operator_.first.length() - 1 < input.length())
						{
							if (operator_.first == input.substr(i, operator_.first.length()))
							{
								found_op = operator_;

								if (operator_.second.first == unary) break;
							}
						}
					}

					tokens.emplace_back(token(found_op.first, found_op.second.first));

					i += found_op.first.length();
				}

			}

			return tokens;
		}
	};

	struct if_node : node
	{
		node* condition;
		node* if_true;

		float calculate() override
		{
			const auto result = condition->calculate();

			if (result != 0) {
				if_true->calculate();
			}

			return 0;
		}
	};

	struct while_node : node
	{
		node* condition;
		node* while_true;

		float calculate() override
		{
			auto result = condition->calculate();

			while (result != 0)
			{
				while_true->calculate();

				result = condition->calculate();
			}

			return 0;
		}
	};

	struct statement_list : node
	{
		vector<node*> nodes;

		float calculate() override
		{
			for (auto& node : nodes)
			{
				node->calculate();
			}

			return 0;
		}
	};

	struct variable_node
	{
		string name;
	};

	struct assignment_node : node
	{
		variable_node* variable;
		node* expression;

		float calculate() override
		{
			variables_[variable->name] = expression->calculate();

			return variables_[variable->name];
		}
	};

	struct math_node : node
	{
		math_tree tree;
		string expression;

		float calculate() override
		{
			tree = math_tree(expression);

			return tree.calculate();
		}
	};
	struct return_node : node
	{
		math_node* value;

		float calculate() override
		{
			returned.push_back(value->calculate());

			return 0;
		}
	};

	node* node_from_string(const string& expression)
	{
		if (expression.find("if") != string::npos)
            if(expression.find("while") != string::npos&& expression.find("while")> expression.find("if")|| expression.find("while") == string::npos)
		{
			const auto first = expression.find('(');

			const auto last = expression.find(')');

			auto* condition = new math_node;

			condition->expression = expression.substr(first, last - first + 1);

			auto* node_if = new if_node;

			statement_list* list;

			if (expression.find('{') != string::npos)
			{
				const auto start = expression.find('{');

				const auto end = expression.find_last_of('}');

				list = build_statement_list(expression.substr(start + 1,
					end - start - 1));
			}
			else
			{
				const auto start = last;

				const auto end = expression.find(';');

				list = build_statement_list(expression.substr(start + 1,
					end - start));
			}

			node_if->condition = condition;

			node_if->if_true = list;

			return node_if;
		}

		if (expression.find("while") != string::npos)
		{
			const auto first = expression.find('(');

			const auto last = expression.find(')');

			auto* condition = new math_node;

			condition->expression = expression.substr(first, last - first + 1);

			auto* node_while = new while_node;

			statement_list* list;

			if (expression.find('{') != string::npos)
			{
				const auto start = expression.find('{');

				const auto end = expression.find_last_of('}');

				list = build_statement_list(expression.substr(start + 1,
					end - start - 1));
			}
			else
			{
				const auto start = last;

				const auto end = expression.find(';');

				list = build_statement_list(expression.substr(start + 1,
					end - start));
			}

			node_while->condition = condition;

			node_while->while_true = list;

			return node_while;
		}

		if (expression.find('=') != string::npos)
		{
			auto* variable = new variable_node;

			const auto name = expression.substr(0, expression.find('='));

			const auto first = name.find_first_not_of(' ');

			const auto last = name.find_last_not_of(' ');

			variable->name = name.substr(first, last - first + 1);

			auto* math = new math_node;

			math->expression = expression.substr(expression.find('=') + 1,
				expression.length() - expression.find('=') - 2);

			auto* assignment = new assignment_node;

			assignment->expression = math;

			assignment->variable = variable;

			return assignment;
		}

		if (expression.find("return") != string::npos)
		{
			auto* math = new math_node;

			math->expression = expression.substr(expression.find('n') + 1,
				expression.length() - expression.find('n') - 2);

			auto* node_return = new return_node;
			node_return->value = math;

			return node_return;
		}

		return nullptr;
	}

	statement_list* build_statement_list(string code)
	{
		auto* statement = new statement_list;

		auto start = 0;

		for (unsigned i = 0; i < code.length(); i++)
		{
			if (code[i] == '{')
			{
				auto bracket = 1;

				while (++i < code.length())
				{
					if (code[i] == '{')
					{
						++bracket;
					}

					else if (code[i] == '}')
					{
						--bracket;
						if (!bracket) break;
					}
				}

				auto* node = node_from_string(code.substr(start,
					i - start + 1));

				statement->nodes.push_back(node);

				start = i + 1;
			}

			else if (code[i] == ';')
			{
				auto* node = node_from_string(code.substr(start,
					i - start + 1));

				statement->nodes.push_back(node);

				start = i + 1;
			}
		}

		return statement;
	}
	void build_tree()
	{
		string code, temporary;

		ifstream handler(file_);

		if (handler.is_open())
		{
			while (getline(handler, temporary))
			{
				code += temporary;
			}

			root_ = build_statement_list(code);
		}

		else cerr << "Can not open file " + file_ << endl;
	}

	statement_list* root_;
};