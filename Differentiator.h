#pragma once

#include <string>
#include <iostream>
#include <sstream>
#include <map>
#include <vector>
#include <iterator>

#include "tree.h"

#define NOT_OPERATOR 33
#define DOUBLE_OPERATOR 10
#define BIN_OPERATOR 100
#define NUM 2
#define VAR 3

#define OPEN_BRACKET '(' //40
#define CLOSE_BRACKET ')' //41


#define OP_MUL '*' //42
#define OP_SUB '-' //45
#define OP_ADD '+' //43
#define OP_DIV '/' //47
#define OP_EXPONENT '^' //94

#define OP_SIN 101
#define OP_COS 102
#define OP_TG  103
#define OP_CTG 104

#define OP_SH  105
#define OP_CH  106
#define OP_TH  107
#define OP_CTH 108

#define OP_SQRT 109


#define $STRING_OPER_TO_COD$ \
{"+", OP_ADD}, \
{"-", OP_SUB}, \
{"*", OP_MUL}, \
{"/", OP_DIV}, \
{"^", OP_EXPONENT}, \
{"sin", OP_SIN}, \
{"cos", OP_COS}, \
{"tg", OP_TG}, \
{"ctg", OP_CTG}, \
{"sh", OP_SH}, \
{"ch", OP_CH}, \
{"th", OP_TH}, \
{"cth", OP_CTH}, \
{"sqrt", OP_SQRT}

#define $COD_TO_OPER_STRING$ \
{OP_ADD, "+"}, \
{OP_SUB, "-"}, \
{OP_MUL, "*"}, \
{OP_DIV, "/"}, \
{OP_EXPONENT, "^"}, \
{OP_SIN, "sin"}, \
{OP_COS, "cos"}, \
{OP_TG, "tg"}, \
{OP_CTG, "ctg"}, \
{OP_SH, "sh"}, \
{OP_CH, "ch"}, \
{OP_TH, "th"}, \
{OP_CTH, "cth"}, \
{OP_SQRT, "sqrt"}

struct element {
    int8_t flag;
    double value;
};

class Expression {
public:
    Expression();
    explicit Expression(std::istream& stream);

public:
    void ReadExpression(std::istream& stream);
    void OutExpressionTree(std::ostream& stream) const;
    void OutExpression(std::ostream& stream) const;
    void OutExpressionTeX(std::ostream& stream) const;
    void clear();
    int64_t calculate(std::map<std::string, int> _vars) const;
    void differentiation(const std::string& var_name);
    void simplification();

private:
    std::vector<element> _preprocessor_for_read_(std::istream& stream);
    Node<element>* _read_(std::vector<element>& _str);
    Node<element>* _read_num_or_var_case_(std::vector<element>::iterator it_begin, std::vector<element>::iterator it_end);
    Node<element>* _read_add_or_sub_case_(std::vector<element>::iterator it_begin, std::vector<element>::iterator it_end);
    Node<element>* _read_mul_or_div_case_(std::vector<element>::iterator it_begin, std::vector<element>::iterator it_end, Node<element>* _left);
    Node<element>* _read_exponent_case_  (std::vector<element>::iterator it_begin, std::vector<element>::iterator it_end);
    Node<element>* _read_binary_case_    (std::vector<element>::iterator it_begin, std::vector<element>::iterator it_end);
    Node<element>* _read_brackets_case_  (std::vector<element>::iterator it_begin, std::vector<element>::iterator it_end);
    std::vector<element>::iterator _find_closing_bracket_(std::vector<element>::iterator it_begin, std::vector<element>::iterator it_end);

    void LNR_for_output_tree(Node<element>* nod, std::ostream& stream) const;
    void LNR_for_output_without_br(Node<element>* nod, std::ostream& stream) const;
    void _for_output_TeX_(Node<element>* nod, std::ostream& stream) const;
    element _coder_operator_(const std::string& _str);

    std::string _decoder_operator_(int _cod);
    uint32_t _FixationVarName_(const std::vector<std::string>& _vars, const std::string& _value);

    Node<element>* _diff_(Node<element>* nod, int var_cod = -1);
    Node<element>* _diff_case_lin_(Node<element>* nod); //'+' and '-'
    Node<element>* _diff_case_mul_(Node<element>* nod);
    Node<element>* _diff_case_div_(Node<element>* nod);
    Node<element>* _diff_case_const(Node<element>* nod);
    Node<element>* _diff_case_var_(Node<element>* nod);

    void _simpler_node_(Node<element>* nod);
    void _simpler_case_mul_(Node<element>* nod);
    void _simpler_case_div_(Node<element>* nod);
    void _simpler_case_add_(Node<element>* nod);
    void _simpler_case_sub_(Node<element>* nod);

    static std::string _get_str_(const std::string& _str_orig, const std::string _delim, size_t _pos = 0);
    static std::string _get_str_(std::istream& stream, const std::string& _delim = " \n\t");
    static bool _in_ (char _symbol, const std::string& _delim);
    static bool _is_digit_(const std::string& _str, double& _result);

private:
    Node<element>* root_orig;
    std::vector<std::string> var_data;
    std::map<std::string, int> oper_to_cod = {$STRING_OPER_TO_COD$};
    std::map<int, std::string> cod_to_oper = {$COD_TO_OPER_STRING$};
    std::map<int, int> oper_to_priority = { {OP_ADD, 1}, {OP_SUB, 1}, {OP_MUL, 2}, {OP_DIV, 2}, {OP_EXPONENT, 3} };
    std::string _special_symbol_ = { "+-*^/()[]" };
};



