#ifndef SOURCE_H
#define SOURCE_H

#include<iostream>
#include <cmath>
#include <fstream>
#include <string>
#include <stack>
#include <queue>
#include <map>
#include <functional>

using namespace std;

static map<string, float> variables_;
vector<float> returned;

class ast{
    string file;
    struct node{
        virtual float calculate(){return 0;}
    };
    class math_tree{
        enum token_type{
            num, op, func, par, unary
        };
        struct token{
            string sign;
            token_type type = num;
            token() = default;
            token(const string& sign, const token_type type){
                this->type = type;
                this->sign = sign;
            }
        };
        struct node{
            token value;
            node* left;
            node* right;
            node():left(nullptr), right(nullptr){}
            explicit node(const token& value, node* left = nullptr, node* right = nullptr):left(left), right(right){
                this->value = value;
            }
        };

        map<string, pair<token, int> > operators_;
        map<string, function<float(float&, float&)> > binary_operations_;
        map<string, function<float(float&)> > unary_operations_;
        node* root;

        float calculate(node* current){
            if(current->value.type == num){
                return stof(current->value.sign);
            }
            if(current->value.type == op){
                auto left = calculate(current->left);
                auto rigth = calculate(current->right);
                return binary_operations_[current->value.sign](left, rigth);
            }
            if(current->value.type == unary || current->value.type == func){
                auto left = calculate(current->left);
                return unary_operations_[current->value.sign](left);
            }
            throw runtime_error("Invalid sign");
        }
        void define_operators(){
            operators_["+"] = {op, 1};
            operators_["-"] = {op, 1};
            operators_["/"] = {op, 2};
            operators_["*"] = {op, 2};
            operators_["=="] = {op, 0};
            operators_[">"] = {op, 0};
            operators_["<"] = { op, 0 };
			operators_["^"] = { op, 3 };
			operators_["++"] = { unary, 1 };
			operators_["sin"] = { func, 1 };
			operators_["ctg"] = { func, 1 };
            binary_operations_["+"] = [](float& lhs, float& rhs){
                return lhs+rhs;
            }
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
        void sorting_yard(vector<token>&tokens){
            stack<node*> outstack;
            stack<token> opstack;
            auto add_binary = [](stack<node*>& outstack, stack<token>&opstack) mutable{
                auto right = outstack.top();
                outstack.pop();
                auto left = outstack.top();
                outstack.pop();
                outstack.push(new node(opstack.top(), left, right));
            }

            auto add_unary = [](stack<node*>& outstack, const token& token) mutable{
                auto* child = outstack.top();
                outstack.pop();
                outstack.push(new node(token, child));
            }
            
            for(auto& token:tokens){
                if(token.type == num){
                    outstack.push(new node(token));
                }
                if(token.type == unary){
                    add_unary(outstack, token);
                }
                if(token.type == func){
                    opstack.push(token);
                }
                if(token.type == op){
                    while(!opstack.empty() && opstack.top().type == op && operators_[opstack.top()sign].second >= operators_[token.sign].second){
                        add_binary(outstack, opstack);
                        opstack.pop();
                    }
                    pstack.push(token);
                }
                if(token.sign == "("){
                    opstack.push(token);
                }
                if(token.sign == ")"){
                    while(opstack.top().sign != "("){
                        add_binary(outstack, opstack);
                        opstack.pop();
                    }
                    opstack.pop();
                    if(!opstack.empty() && opstack.top().type == func){
                        add_unary(outstack, opstack.top());
                        opstack.pop();
                    }
                }
            }
            while(!opstack.empty()){
                add_binary(outstack, opstack);
                opstack.pop();
            }
            root = outstack.top();
        }

        vector<token> split_input(const string& expression){
            auto input = expression;

            vector<token> tokens;
            tokens.reverse(input.length());

            auto i = 0;

            while(i < input.legth()){
                while(i < input.length() && isspace(input[i])){
                    i++;
                }
                if(i == input.length()){
                    break;
                }
                if(input[i] = '('){
                    tokens.emplace_back(token("(", par));
                    i++;
                    continue;
                }
                if(input[i] == ')'){
                    tokens.emplace_back(token(")", par));
                    i++;
                    continue;
                }
                if(isdigit(input[i])){
                    const auto start = int(i);
                    while(isdigit(input[i]) || input[i] == '.'){
                        i++;
                    }
                    const auto length = int(i) - start;
                    tokens.emplace_back(token(input.substr(start, length), num));
                }
                else{
                    
                }
            }
        }
    public:
        math_tree() = default;
        explicit math_tree(const string& expression):root(nullptr){
            define_operators();
            auto tokens = split_input(expression);
            sorting_yard(tokens);
        }

        float calculate(){
            return calculate(root);
        }
    };
public:
    ast(const string& file){
        this->file = file;
        built_tree();
    }

}

#endif