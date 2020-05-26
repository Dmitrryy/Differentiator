#include "Differentiator.h"

#include <cstring>
#include <algorithm>
#include <cassert>
#include <deque>
#include <cmath>

#define Node Node<element>
#define $lp left->parent
#define $rp right->parent
#define $flag value.flag
#define $value value.value

#define $LOG$ 1
//#define $DETAILED_LOG$ 2
static size_t counter = 0;

Expression::Expression()
: root_orig(nullptr)
, num_name(counter++)
{
#ifdef $LOG$
    makeLogEntry("*(Expression #" + std::to_string(num_name) + ") Entry default constructor", __LINE__);
#endif
}

Expression::Expression(const std::string& _str)
: num_name(counter++)
, root_orig(nullptr)
{
#ifdef $LOG$
    makeLogEntry("*(Expression #" + std::to_string(num_name) + ") Entry constructor 'Expression(const std::string &)'", __LINE__);
#endif
    std::stringstream in(_str);
    ReadExpression(in);
}

Expression::Expression(std::istream &stream)
: root_orig(nullptr)
, num_name(counter++)
{
#ifdef $LOG$
    makeLogEntry("*(Expression #" + std::to_string(num_name) + ") Entry constructor 'Expression(std::istream &)'", __LINE__);
#endif
    ReadExpression(stream);
}

Expression::Expression(const Expression &that)
: root_orig(Tree_copy(that.root_orig))
, var_values(that.var_values)
, var_data(that.var_data)
, num_name(counter++)
{
#ifdef $LOG$
    makeLogEntry("*(Expression #" + std::to_string(num_name) + ") Entry copy constructor. Copy of (Expression #"
                    + std::to_string(that.num_name) + ")", __LINE__);
#endif
}

Expression::Expression(Expression&& that)
: root_orig(that.root_orig)
, var_data(std::move(that.var_data))
, var_values(std::move(that.var_values))
, num_name(counter++)
{
#ifdef $LOG$
    makeLogEntry("*(Expression #" + std::to_string(num_name) + ") Entry move constructor", __LINE__);
#endif
    that.root_orig = nullptr;
}

Expression::~Expression() {
#ifdef $LOG$
    makeLogEntry("*(Expression #" + std::to_string(num_name) + ") Entry destructor", __LINE__);
#endif
    delete root_orig;
}


Expression &Expression::operator=(Expression that) {
#ifdef $LOG$
    makeLogEntry("operator '=': (Expression #" + std::to_string(num_name) + ") = (" + "(Expression #" + std::to_string(that.num_name) + ")", __LINE__);
#endif

    std::swap(root_orig, that.root_orig);
    var_data = std::move(that.var_data);
    var_values = std::move(that.var_values);

    return *this;
}
Expression &Expression::operator+=(const Expression &that) {
#ifdef $LOG$
    makeLogEntry("operator '+=': (Expression #" + std::to_string(num_name) + ") += (" + "(Expression #" + std::to_string(that.num_name) + ")", __LINE__);
#endif
    if (that.root_orig != nullptr) {
        root_orig = CreateNode(element{DOUBLE_OPERATOR, OP_ADD}, root_orig, Tree_copy(that.root_orig));

        NLR(root_orig->right, [&](Node* nod) {
            if (nod->$flag == VAR) {
                nod->$value = _FixationVarName_(var_data, that.var_data[nod->$value]);
            }
        });
    }

    return *this;
}
Expression operator+(const Expression& lhs, const Expression& rhs) {
#ifdef $LOG$
    makeLogEntry("operator '+(const &, const &)': (Expression #" + std::to_string(lhs.num_name) + ") + (" + "(Expression #" + std::to_string(rhs.num_name) + ")", __LINE__);
#endif

    Expression tmp(lhs);
    tmp += rhs;

    return tmp;
}
Expression operator+(Expression&& lhs, const Expression& rhs) {
#ifdef $LOG$
    makeLogEntry("operator '+( &&, const &)': (Expression #" + std::to_string(lhs.num_name) + ") + (" + "(Expression #" + std::to_string(rhs.num_name) + ")", __LINE__);
#endif

    lhs += rhs;
    return std::move(lhs);
}

void Expression::ReadExpression(std::istream &stream) {
#ifdef $LOG$
        makeLogEntry("(Expression #" + std::to_string(num_name) + ") Entry ReadExpression(std::istream &stream)", __LINE__);
#endif
    delete root_orig;

    std::vector<element> exp = _preprocessor_for_read_(stream);
    root_orig = _read_(exp);

    UpValueVar({});
}
std::vector<element> Expression::_preprocessor_for_read_(std::istream &stream) {
#ifdef $DETAILED_LOG$
    makeLogEntry("(Expression #" + std::to_string(num_name) + ") Entry _preprocessor_for_read_(std::istream &stream)", __LINE__);
#endif
    std::vector<element> result;

    char current_sign = stream.peek();
    while (stream) {
        if (current_sign == ' ' || current_sign == '\t' || current_sign == '\n') {
            /*nop()*/;
            stream.ignore(1);
        }
        else if (current_sign == '(' || current_sign == ')') {
            result.push_back( {SPECIAL_CHARACTER, static_cast<double>(current_sign)} );
            stream.ignore(1);
        } else if (_in_(current_sign, "+-*/^")) {
            result.push_back({DOUBLE_OPERATOR, static_cast<double>(current_sign)});
            stream.ignore(1);
        }
        else {
            std::string str_elem = _get_str_(stream, " +-*/^()[]\t\n");

            if (!str_elem.empty()) {
                element cod_elem = _coder_operator_(str_elem);
                if (cod_elem.flag == SPECIAL_CHARACTER) {

                    double num = 0;
                    if (_is_digit_(str_elem, num)) {
                        cod_elem = {NUM, num};
                    } else {
                        cod_elem.flag = VAR;
                        cod_elem.value = _FixationVarName_(var_data, str_elem);
                    }
                }
                result.push_back(cod_elem);
            }
        }
        current_sign = stream.peek();
    }
    return result;
}
std::string Expression::_get_str_(std::istream &stream, const std::string &_delim) {

    std::string result;
    char _symbol = 0;
    while (stream.get(_symbol)) {
        if (_in_(_symbol, _delim)) {
            stream.seekg(-1, std::ios_base::cur);
            break;
        }
        result.push_back(_symbol);
    }

    return result;
}
bool Expression::_in_(char _symbol,const std::string& _delim) {
    for(const auto& x : _delim) {
        if (_symbol == x) {
            return true;
        }
    }
    return false;
}
bool Expression::_is_digit_(const std::string &_str, double &_result) {

    double num = std::strtod(_str.c_str(), nullptr);

    if (num != 0 || _str[0] == '0' || (_str[0] == '-' || _str[0] == '+') && _str[1] == '0') {
        _result = num;
        return true;
    }
    return false;
}

uint32_t Expression::_FixationVarName_(const std::vector<std::string>& _vars, const std::string& _value) {
#ifdef $DETAILED_LOG$
    makeLogEntry("(Expression #" + std::to_string(num_name) + ") Entry _FixationVarName_(const std::vector<std::string>& _vars, const std::string& _value)", __LINE__);
#endif

    auto it = std::find(_vars.begin(), _vars.end(), _value);
    if (it != var_data.end()) {
        return it - var_data.begin();
    }
    else {
        if (var_data.empty()) {
            var_data.resize(2);
            var_data[1] = _value;
        }
        else {
            var_data.push_back(_value);
        }
    }
    return _vars.size() - 1;
}

element Expression::_coder_operator_(const std::string &_str) {
#ifdef $DETAILED_LOG$
    makeLogEntry("(Expression #" + std::to_string(num_name) + ") Entry _coder_operator_(const std::string &_str)", __LINE__);
#endif
    auto it = oper_to_cod.find(_str);

    if (it == oper_to_cod.end()) {
        return {SPECIAL_CHARACTER, 0};
    }
    if (it->second >= BIN_OPERATOR) { //номер бинарной операции начинается со 100
        return {BIN_OPERATOR, static_cast<double>(it->second)};
    }
    return {DOUBLE_OPERATOR, static_cast<double>(it->second)};
}


Node *Expression::_read_(std::vector<element>& _str) {
#ifdef $DETAILED_LOG$
    makeLogEntry("(Expression #" + std::to_string(num_name) + ") Entry _read_(std::vector<element>& _str)", __LINE__);
#endif
    if (_str.empty()) {
        return nullptr;
    }
    auto it_begin = _str.begin();
    auto it_end = _str.end() - 1;
    Node* result = nullptr;

    if (_str[0].flag == NUM || _str[0].flag == VAR) {
        result = _read_num_or_var_case_(it_begin, it_end);
    }
    else if (_str[0].flag == BIN_OPERATOR) {
        result = _read_binary_case_(it_begin, it_end);
    }
    else if (_str[0].flag == SPECIAL_CHARACTER && _str[0].value == OPEN_BRACKET) {
        result = _read_brackets_case_(it_begin, it_end);
    }
    else if (it_begin->flag == DOUBLE_OPERATOR && (it_begin->value == OP_SUB || it_begin->value == OP_ADD)) {
        result = _read_add_or_sub_case_(it_begin, it_end, true);
        assert(result != nullptr);
    }
    else {
        throw std::invalid_argument("non-violent record of expression");
    }
    return result;
}
Node* Expression::_read_num_or_var_case_(std::vector<element>::iterator it_begin, std::vector<element>::iterator it_end) {
#ifdef $DETAILED_LOG$
    makeLogEntry("(Expression #" + std::to_string(num_name) + ") Entry _read_num_or_var_case_", __LINE__);
#endif

    Node* num = CreateNode(*it_begin);
    Node* result = nullptr;

    auto new_begin = it_begin;
    if (it_begin != it_end) {
        new_begin++;
    } else {
        return num;
    }

    if (new_begin->flag == DOUBLE_OPERATOR) {
        if (new_begin->value == OP_ADD || new_begin->value == OP_SUB) {
            result = _read_add_or_sub_case_(new_begin, it_end);
            assert(result != nullptr);

            Tree_connect(result, num);
            num = nullptr;
        }
        else if (new_begin->value == OP_MUL || new_begin->value == OP_DIV) {
            result = _read_mul_or_div_case_(new_begin, it_end, num);
            assert(result != nullptr);

            num = nullptr;
        }
        else if (new_begin->value == OP_EXPONENT) {
            result = _read_exponent_case_(new_begin, it_end, num);
            assert(result != nullptr);
        }
        else { assert(0); }
    }
    else if (new_begin->flag == SPECIAL_CHARACTER && new_begin->value == CLOSE_BRACKET) {
        result = num;
        num = nullptr;
    }
    else { throw std::runtime_error("Expected 'DOUBLE_OPERATOR' or 'CLOSE_BRACKET' but received: " + std::to_string(new_begin->flag)); }

    return result;
}
Node* Expression::_read_add_or_sub_case_(std::vector<element>::iterator it_begin, std::vector<element>::iterator it_end, bool _unary) {
#ifdef $DETAILED_LOG$
    makeLogEntry("(Expression #" + std::to_string(num_name) + ") Entry _read_add_or_sub_case_", __LINE__);
#endif

    Node* result = CreateNode(*it_begin);
    Node* num = nullptr;

    if (_unary) {
        result->$flag = BIN_OPERATOR;
        result->$value = (result->$value == OP_ADD) ? UN_PLUS : UN_MINUS;
    }

    auto new_begin = it_begin;
    if (it_begin != it_end) {
        new_begin++;
    } else {
        throw std::runtime_error("Expression ends with Double operator.");
    }

    if (new_begin->flag == NUM || new_begin->flag == VAR) {
        Tree_connect(result, (Node*) nullptr, _read_num_or_var_case_(new_begin, it_end));

        assert(result->right != nullptr);
    }
    else if (new_begin->flag == SPECIAL_CHARACTER && new_begin->value == OPEN_BRACKET) {
        Tree_connect(result, (Node*) nullptr, _read_brackets_case_(new_begin, it_end));

        assert(result->right != nullptr);
    }
    else if (new_begin->flag == BIN_OPERATOR) {
        Tree_connect(result, (Node*) nullptr, _read_binary_case_(new_begin, it_end));

        assert(result->right != nullptr);
    }
    else { throw std::runtime_error("in _read_add_or_sub_case_, received: " + std::to_string(new_begin->flag)); }

    return result;
}
Node* Expression::_read_mul_or_div_case_(std::vector<element>::iterator it_begin, std::vector<element>::iterator it_end, Node* _left) {
#ifdef $DETAILED_LOG$
    makeLogEntry("(Expression #" + std::to_string(num_name) + ") Entry _read_mul_or_div_case_", __LINE__);
#endif

    Node* result = CreateNode(*it_begin, _left);
    Node* num = nullptr;

    auto new_begin = it_begin;
    if (it_begin != it_end) {
        new_begin++;
    } else {
        throw std::runtime_error("Expression ends with Double operator.");
    }

    if (new_begin->flag == NUM || new_begin->flag == VAR) {
        Node* tmp = _read_num_or_var_case_(new_begin, it_end);
        assert(tmp != nullptr);

        if (tmp->$flag == DOUBLE_OPERATOR && (tmp->$value == OP_ADD || tmp->$value == OP_SUB)) {
            Tree_connect(result, (Node*) nullptr, tmp->left);
            Tree_connect(tmp, result);
            result = tmp;
        }
        else {
            Tree_connect(result, (Node*) nullptr, tmp);
        }
    }
    else if (new_begin->flag == SPECIAL_CHARACTER && new_begin->value == OPEN_BRACKET) {
        auto new_end = _find_closing_bracket_(new_begin, it_end + 1);
        assert(new_end != it_end + 1);

        Tree_connect(result, (Node*) nullptr, _read_brackets_case_(new_begin, new_end));
        assert(result->right != nullptr);

        //после скобок
        if (new_end != it_end) {
            new_begin = new_end + 1;

            if (new_begin->flag == DOUBLE_OPERATOR) {
                if (new_begin->value == OP_ADD || new_begin->value == OP_SUB) {
                    Node* tmp = _read_add_or_sub_case_(new_begin, it_end);
                    assert(tmp != nullptr);

                    Tree_connect(tmp, (Node*) nullptr, result);
                    result = tmp;
                    tmp = nullptr;
                }
                else if (new_begin->value == OP_MUL || new_begin->value == OP_DIV) {
                    result = _read_mul_or_div_case_(new_begin, it_end, result);
                    assert(result != nullptr);
                }
                else if (new_begin->value == OP_EXPONENT) {
                    Tree_connect(result, (Node*) nullptr, _read_exponent_case_(new_begin, it_end, result->right));
                }
                else { throw std::runtime_error("in bracket case(after) value:" + std::to_string(new_begin->value)); }
            }else { throw std::runtime_error("in bracket case(after) flag:" + std::to_string(new_begin->flag)); }
        }
    }
    else if (new_begin->flag == BIN_OPERATOR) {
        Node* tmp = _read_binary_case_(new_begin, it_end);
        assert(tmp != nullptr);

        if (tmp->$flag == DOUBLE_OPERATOR && (tmp->$value == OP_ADD || tmp->$value == OP_SUB)) {
            Tree_connect(result, (Node*) nullptr, tmp->left);
            Tree_connect(tmp, result);
            result = tmp;
        }
        else {
            Tree_connect(result, (Node*) nullptr, tmp);
        }
    }
    else { throw std::runtime_error("in _read_add_or_sub_case_, received: " + std::to_string(new_begin->flag)); }

    return result;
}
Node* Expression::_read_brackets_case_  (std::vector<element>::iterator it_begin, std::vector<element>::iterator it_end) {
#ifdef $DETAILED_LOG$
    makeLogEntry("(Expression #" + std::to_string(num_name) + ") Entry _read_brackets_case_", __LINE__);
#endif

    auto new_begin = it_begin;
    if (it_begin != it_end) {
        new_begin++;
    } else {
        throw std::runtime_error("Expression ends with open bracket.");
    }

    std::vector<element>::iterator end_bracket = _find_closing_bracket_(it_begin, it_end + 1);

    if (end_bracket == it_end + 1 /*&& end_bracket->flag != SPECIAL_CHARACTER && end_bracket->value != CLOSE_BRACKET*/) {
        throw std::runtime_error("Open bracket.");
    }
    end_bracket--;

    //внутри скобок
    Node* inside_brackets = nullptr;
    if (new_begin->flag == NUM || new_begin->flag == VAR) {
        inside_brackets = _read_num_or_var_case_(new_begin, end_bracket);
        assert(inside_brackets != nullptr);
    }
    else if (new_begin->flag == BIN_OPERATOR) {
        inside_brackets = _read_binary_case_(new_begin, end_bracket);
        assert(inside_brackets != nullptr);
    }
    else if (new_begin->flag == SPECIAL_CHARACTER && new_begin->value == OPEN_BRACKET) {
        inside_brackets = _read_brackets_case_(new_begin, end_bracket);
        assert(inside_brackets != nullptr);
    }
    else if (new_begin->flag == DOUBLE_OPERATOR && (new_begin->value == OP_SUB || new_begin->value == OP_ADD)) {
        inside_brackets = _read_add_or_sub_case_(new_begin, end_bracket, true);
        assert(inside_brackets != nullptr);
    }
    else { throw std::runtime_error("in bracket case(inside):" + std::to_string(new_begin->flag)); }

    //после скобок
    new_begin = end_bracket + 1;
    Node* result = nullptr;
    if (new_begin == it_end) {
        result = inside_brackets;
    }
    else {
        new_begin++;
        if (new_begin->flag == DOUBLE_OPERATOR) {
            if (new_begin->value == OP_ADD || new_begin->value == OP_SUB) {
                result = _read_add_or_sub_case_(new_begin, it_end);
                assert(result != nullptr);

                if (result->left == nullptr) {
                    result->left = inside_brackets;
                } else {
                    result->right = inside_brackets;
                }
                inside_brackets = nullptr;
            } else if (new_begin->value == OP_MUL || new_begin->value == OP_DIV) {
                result = _read_mul_or_div_case_(new_begin, it_end, inside_brackets);
                assert(result != nullptr);

                inside_brackets = nullptr;
            } else if (new_begin->value == OP_EXPONENT) {
                result = _read_exponent_case_(new_begin, it_end, inside_brackets);
                assert(result != nullptr);

                inside_brackets = nullptr;
            }
            else { throw std::runtime_error("in bracket case(after) value:" + std::to_string(new_begin->value)); }
        }else { throw std::runtime_error("in bracket case(after) flag:" + std::to_string(new_begin->flag)); }
    }
    return result;
}
Node* Expression::_read_binary_case_ (std::vector<element>::iterator it_begin, std::vector<element>::iterator it_end) {
#ifdef $DETAILED_LOG$
    makeLogEntry("(Expression #" + std::to_string(num_name) + ") Entry _read_binary_case_", __LINE__);
#endif

    Node* bin_operator = CreateNode(*it_begin);

    std::vector<element>::iterator new_begin;
    if (it_begin == it_end) {
        throw std::runtime_error("Expression ends with binary operator(without argument).");
    } else {
        new_begin = it_begin + 1;
    }

    //argument
    std::vector<element>::iterator end_bracket;
    if (new_begin->flag == SPECIAL_CHARACTER && new_begin->value == OPEN_BRACKET) {

        end_bracket = _find_closing_bracket_(new_begin, it_end);
        if (end_bracket == it_end && end_bracket->flag != SPECIAL_CHARACTER && end_bracket->value != CLOSE_BRACKET) {
            throw std::runtime_error("Open bracket.");
        }

        bin_operator->right = _read_brackets_case_(new_begin, end_bracket);
        assert(bin_operator->right != nullptr);
    }
    else { throw std::runtime_error("In binary case. Expected '(', but received: " + std::to_string(new_begin->value)); }

    //after
    new_begin = end_bracket;
    Node* result = nullptr;
    if (new_begin == it_end) {
        result = bin_operator;
        bin_operator = nullptr;
    }
    else {
        new_begin++;
        if (new_begin->flag == DOUBLE_OPERATOR) {
            if (new_begin->value == OP_ADD || new_begin->value == OP_SUB) {
                result = _read_add_or_sub_case_(new_begin, it_end);
                assert(result != nullptr);

                if (result->left == nullptr) {
                    result->left = bin_operator;
                } else {
                    result->right = bin_operator;
                }
                bin_operator = nullptr;
            } else if (new_begin->value == OP_MUL || new_begin->value == OP_DIV) {
                result = _read_mul_or_div_case_(new_begin, it_end, bin_operator);

                bin_operator = nullptr;
            }
        }
        else { throw std::runtime_error("in bracket case(after) flag:" + std::to_string(new_begin->flag)); }
    }

    return result;
}
Node* Expression::_read_exponent_case_  (std::vector<element>::iterator it_begin, std::vector<element>::iterator it_end, Node* _left) {
#ifdef $DETAILED_LOG$
    makeLogEntry("(Expression #" + std::to_string(num_name) + ") Entry _read_exponent_case_", __LINE__);
#endif

    Node* exp = CreateNode(*it_begin, _left);
    Node* result = nullptr;

    auto new_begin = it_begin;
    if (it_begin != it_end) {
        new_begin++;
    } else {
        throw std::runtime_error("Expression ends with Double operator '^'.");
    }

    //exponent
    if (new_begin->flag == NUM || new_begin->flag == VAR) {
        exp->right = CreateNode(*new_begin);
    }
    else if (new_begin->flag == BIN_OPERATOR) {
        auto end_bin = _find_closing_bracket_(new_begin + 1, it_end);

        exp->right = _read_binary_case_(new_begin, end_bin);
        new_begin = end_bin;
    }
    else if (new_begin->flag == SPECIAL_CHARACTER && new_begin->value == OPEN_BRACKET) {
        auto end_br = _find_closing_bracket_(new_begin, it_end);

        exp->right = _read_brackets_case_(new_begin, end_br);
        new_begin = end_br;
    }
    else { throw std::runtime_error("after operator '^' received: flag(" + std::to_string(new_begin->flag)
                                    + "), value(" + std::to_string(new_begin->value) + ")"); }

    //after
    if (new_begin != it_end) {
        new_begin++;

        if (new_begin->flag == DOUBLE_OPERATOR) {
            if (new_begin->value == OP_ADD || new_begin->value == OP_SUB) {
                result = _read_add_or_sub_case_(new_begin, it_end);
                assert(result != nullptr);

                result->left = exp;
                exp = nullptr;
            }
            else if (new_begin->value == OP_MUL || new_begin->value == OP_DIV) {
                result = _read_mul_or_div_case_(new_begin, it_end, exp);
                assert(result != nullptr);
                exp = nullptr;
            }
            else { assert(0); }
        }
        else { assert(0); }
    }
    else {
        result = exp;
    }

    return result;
}
void Expression::clear() {
#ifdef $LOG$
    makeLogEntry("(Expression #" + std::to_string(num_name) + ") Entry clear()", __LINE__);
#endif

    delete root_orig;
    root_orig = nullptr;

    var_data.clear();
}



void Expression::OutExpressionTree(std::ostream &_stream) const {
#ifdef $LOG$
    makeLogEntry("(Expression #" + std::to_string(num_name) + ") Entry OutExpressionTree()", __LINE__);
#endif
    LNR_for_output_tree(root_orig, _stream);
}
void Expression::LNR_for_output_tree(Node *nod, std::ostream &stream) const {
#ifdef $DETAILED_LOG$
    makeLogEntry("(Expression #" + std::to_string(num_name) + ") Entry LNR_for_output_tree", __LINE__);
#endif

    if (nod == nullptr) {
        return ;
    }
    if (nod->left == nullptr && nod->right == nullptr) {

        stream << '[';
        if (nod->$flag == VAR) {
            assert(nod->$value < var_data.size() && nod->$value >= 0);

            stream << var_data.at(nod->$value);
        } else if (nod->$flag == NUM) {
            stream << nod->$value;
        } else {

            throw std::invalid_argument("Expected VAR(" + std::to_string(VAR) + ") or NUM("
                                   + std::to_string(NUM) + "), but received: " + std::to_string(nod->$flag));
        }
        stream << ']';

        return ;
    }

    stream << '(';
    LNR_for_output_tree(nod->left, stream);

    stream << cod_to_oper.at(nod->$value);

    LNR_for_output_tree(nod->right, stream);
    stream << ')';
}

void Expression::OutExpression(std::ostream &stream) const {
#ifdef $LOG$
    makeLogEntry("(Expression #" + std::to_string(num_name) + ") Entry OutExpression", __LINE__);
#endif

    LNR_for_output_without_br(root_orig, stream);
}
void Expression::LNR_for_output_without_br(Node *nod, std::ostream &stream) const {
#ifdef $DETAILED_LOG$
    makeLogEntry("(Expression #" + std::to_string(num_name) + ") Entry LNR_for_output_without_br", __LINE__);
#endif

    if (nod->left == nullptr || nod->right == nullptr) {
        if (nod->left == nullptr && nod->right == nullptr) {
            if (nod->$flag == VAR) {
                assert(nod->$value < var_data.size() && nod->$value >= 0);

                stream << var_data[nod->$value];
            } else if (nod->$flag == NUM) {
                stream << nod->$value;
            } else {

                throw std::invalid_argument("Expected VAR(" + std::to_string(VAR) + ") or NUM("
                                            + std::to_string(NUM) + "), but received: " + std::to_string(nod->$flag));
            }
            return;
        }
        else if (nod->$flag == BIN_OPERATOR) {
            stream << cod_to_oper.at(int(nod->$value)) << '(';
            LNR_for_output_without_br(nod->right, stream);
            stream << ')';

            return ;
        }
        else { assert(0); }
    }

    bool br_is_open = false;
    if (nod->left->$flag == DOUBLE_OPERATOR) {
        if (oper_to_priority.at(nod->$value) > oper_to_priority.at(nod->left->$value)) {
            stream << '(';
            br_is_open = true;
        }
    }
    LNR_for_output_without_br(nod->left, stream);
    if (br_is_open) {
        stream << ')';
        br_is_open = false;
    }

    stream << ' ' << cod_to_oper.at(int(nod->$value)) << ' ';


    if (nod->right->$flag == DOUBLE_OPERATOR) {
        if (oper_to_priority.at(nod->$value) > oper_to_priority.at(nod->right->$value)) {
            stream << '(';
            br_is_open = true;
        }
    }
    LNR_for_output_without_br(nod->right, stream);
    if (br_is_open) {
        stream << ')';
        br_is_open = false;
    }
}

void Expression::OutExpressionTeX(std::ostream &stream) const {
#ifdef $LOG$
    makeLogEntry("(Expression #" + std::to_string(num_name) + ") Entry OutExpressionTeX", __LINE__);
#endif

    _for_output_TeX_(root_orig, stream);
}
void Expression::_for_output_TeX_(Node* nod, std::ostream &stream) const {
#ifdef $DETAILED_LOG$
    makeLogEntry("(Expression #" + std::to_string(num_name) + ") Entry _for_output_TeX_", __LINE__);
#endif

    if (nod == nullptr) {
        return ;
    }
    if (nod->$flag == NUM) {
        stream << nod->$value;
    }
    else if (nod->$flag == VAR) {
        stream << var_data[nod->$value];
    }
    else if (nod->$flag == DOUBLE_OPERATOR) {

        if (nod->$value == OP_MUL || nod->$value == OP_ADD || nod->$value == OP_SUB) {
            bool br_is_open = false;

            if (nod->left->$flag == DOUBLE_OPERATOR && oper_to_priority.at(nod->$value) > oper_to_priority.at(nod->left->$value)
                || (nod->left->$flag == BIN_OPERATOR && nod->left->$value == UN_MINUS)) {
                stream << "\\left(";
                br_is_open = true;
            }
            _for_output_TeX_(nod->left, stream);
            if (br_is_open) {
                stream << "\\right)";
                br_is_open = false;
            }

            stream << cod_to_oper.at(nod->$value);

            if (nod->right->$flag == DOUBLE_OPERATOR && oper_to_priority.at(nod->$value) > oper_to_priority.at(nod->right->$value)) {
                stream << "\\left(";
                br_is_open = true;
            }
            _for_output_TeX_(nod->right, stream);
            if (br_is_open) {
                stream << "\\right)";
                br_is_open = false;
            }
        }
        else if (nod->$value == OP_DIV) {
            stream << "\\frac{";
            _for_output_TeX_(nod->left, stream);
            stream << "}{";
            _for_output_TeX_(nod->right, stream);
            stream << "} ";
        }
        else if (nod->$value == OP_EXPONENT) {
            if (nod->left->$flag == NUM || nod->left->$flag == VAR) {
                _for_output_TeX_(nod->left, stream);

                stream << cod_to_oper.at(OP_EXPONENT) << '{';
                _for_output_TeX_(nod->right, stream);
                stream << '}';
            }
            else if (nod->left->$flag == BIN_OPERATOR) {
                if (nod->left->$value == OP_SQRT) {
                    stream << "\\left(";
                    _for_output_TeX_(nod->left, stream);
                    stream << "\\right) ^ {";
                    _for_output_TeX_(nod->right, stream);
                    stream << '}';
                }
                else if (nod->left->$value == UN_MINUS) {
                    stream << "\\left(";
                    _for_output_TeX_(nod->left, stream);
                    stream << "\\right) ^ {";
                    _for_output_TeX_(nod->right, stream);
                    stream << '}';
                }
                else {
                    stream << '\\' << cod_to_oper.at(nod->left->$value);
                    stream << "^{";
                    _for_output_TeX_(nod->right, stream);
                    stream << '}';

                    stream << "\\left(";
                    _for_output_TeX_(nod->left->right, stream);
                    stream << "\\right)";
                }
            }
            else {
                stream << "\\left(";
                _for_output_TeX_(nod->left, stream);
                stream << "\\right)^{";
                _for_output_TeX_(nod->right, stream);
                stream << '}';
            }
        }
    } else if (nod->$flag == BIN_OPERATOR) {
        if (nod->$value == OP_SQRT) {
            stream << "\\sqrt{";
            _for_output_TeX_(nod->right, stream);
            stream << '}';
        }
        else if (nod->$value == UN_MINUS) {

            if (nod->right->$flag == NUM || nod->right->$flag == VAR) {
                stream << '-';
                _for_output_TeX_(nod->right, stream);
            }
            else {
                stream << "-\\left(";
                _for_output_TeX_(nod->right, stream);
                stream << "\\right)";
            }

        }
        else {
            stream << "\\" << cod_to_oper.at(nod->$value) << "\\left(";
            _for_output_TeX_(nod->right, stream);
            stream << "\\right)";
        }
    } else { assert(0); }


}

std::vector<element>::iterator
Expression::_find_closing_bracket_(std::vector<element>::iterator it_begin, std::vector<element>::iterator it_end) {
#ifdef $DETAILED_LOG$
    makeLogEntry("(Expression #" + std::to_string(num_name) + ") Entry _find_closing_bracket_", __LINE__);
#endif

    std::vector<element>::iterator end_bracket;

    int count_brackets = 0; //считая первую скобку
    end_bracket = std::find_if(it_begin, it_end, [&count_brackets](const element &x) {
        if (x.flag == SPECIAL_CHARACTER && x.value == OPEN_BRACKET) {
            count_brackets++;
        }
        if (x.flag == SPECIAL_CHARACTER && x.value == CLOSE_BRACKET) {
            count_brackets--;
        }
        return count_brackets == 0;
    });

    return end_bracket;
}

double Expression::calculate(const std::map<std::string, int> &_vars) {
#ifdef $LOG$
    makeLogEntry("(Expression #" + std::to_string(num_name) + ") Entry calculate", __LINE__);
#endif
    UpValueVar(_vars);

    return calculate();
}
void Expression::UpValueVar(const std::map<std::string, int> &_vars) {
#ifdef $LOG$
    makeLogEntry("(Expression #" + std::to_string(num_name) + ") Entry UpValueVar", __LINE__);
#endif

    int i = 0;
    for (const auto& var : var_data) {
        auto it = _vars.find(var);

        if (it != _vars.end()) {
            var_values[i] = it->second;
        }
        else {
            var_values[i];
        }
        i++;
    }
}
double Expression::calculate() const {
#ifdef $LOG$
    makeLogEntry("(Expression #" + std::to_string(num_name) + ") Entry calculate", __LINE__);
#endif

    return _rec_calculate_(root_orig);
}
double Expression::_rec_calculate_(Node* root) const {
#ifdef $DETAILED_LOG$
    makeLogEntry("(Expression #" + std::to_string(num_name) + ") Entry _rec_calculate_", __LINE__);
#endif

    if (root == nullptr) {
        return 0;
    }

    double result = 0;

    if (root->$flag == NUM) {
        result = root->$value;
    }
    else if (root->$flag == VAR) {
        result = var_values.at(root->$value);
    }
    else if (root->$flag == DOUBLE_OPERATOR) {

        if (root->$value == OP_SUB) {
            result = _rec_calculate_(root->left) - _rec_calculate_(root->right);
        }
        else if (root->$value == OP_ADD) {
            result = _rec_calculate_(root->left) + _rec_calculate_(root->right);
        }
        else if (root->$value == OP_MUL) {
            result = _rec_calculate_(root->left) * _rec_calculate_(root->right);
        }
        else if (root->$value == OP_DIV) {
            double left = _rec_calculate_(root->left);
            double right = _rec_calculate_(root->right);

            if (right != 0) {
                result = left / right;
            }
            else {
                std::ostringstream err;
                OutExpression(err);
                throw std::runtime_error("Division by zero!\n in: " + err.str());
            }
        }
        else if (root->$value == OP_EXPONENT) {
            result = std::pow(_rec_calculate_(root->left), _rec_calculate_(root->right));
        }
        else { assert(0); }
    }
    else if (root->$flag == BIN_OPERATOR) {
        if (root->$value == OP_SIN) {
            result = std::sin(_rec_calculate_(root->right));
        }
        else if (root->$value == OP_COS) {
            result = std::cos(_rec_calculate_(root->right));
        }
        else if (root->$value == OP_TG) {
            result = std::tan(_rec_calculate_(root->right));
        }
        else if (root->$value == OP_CTG) {
            result = 1 / std::tan(_rec_calculate_(root->right));
        }
        else if (root->$value == OP_ASIN) {
            result = std::asin(_rec_calculate_(root->right));
        }
        else if (root->$value == OP_ACOS) {
            result = std::acos(_rec_calculate_(root->right));
        }
        else if (root->$value == OP_ATG) {
            result = std::atan(_rec_calculate_(root->right));
        }
        else if (root->$value == OP_SH) {
            result = std::sinh(_rec_calculate_(root->right));
        }
        else if (root->$value == OP_CH) {
            result = std::cosh(_rec_calculate_(root->right));
        }
        else if (root->$value == OP_TH) {
            result = std::tanh(_rec_calculate_(root->right));
        }
        else if (root->$value == OP_SQRT) {
            result = std::sqrt(_rec_calculate_(root->right));
        }
        else if (root->$value == OP_LN) {
            result = std::log(_rec_calculate_(root->right));
        }
        else { assert(0); }
    }
    return result;
}

Expression Expression::
differentiation(const std::string &var_name) {
#ifdef $LOG$
    makeLogEntry("(Expression #" + std::to_string(num_name) + ") Entry differentiation", __LINE__);
#endif

    auto it = std::find(var_data.begin(), var_data.end(), var_name);

    if (it == var_data.end()) {
        throw std::invalid_argument("Unknown variable: \" + var_name");
    }
    Expression res;
    res.root_orig = _diff_(root_orig, it - var_data.begin());
    res.var_data = var_data;
    res.var_values = var_values;

    return res;
}

Node* Expression::_diff_(Node* nod, int var_cod) {
#ifdef $DETAILED_LOG$
    makeLogEntry("(Expression #" + std::to_string(num_name) + ") Entry _diff_", __LINE__);
#endif

    Node* result = nullptr;

    if (nod->$flag == NUM) {
        result = CreateNode(element{NUM, 0});
    }
    else if (nod->$flag == VAR) {
        result = CreateNode(element{NUM, double(nod->$value == var_cod)});
    }
    else if (nod->$flag == DOUBLE_OPERATOR) {
        if (nod->$value == OP_ADD || nod->$value == OP_SUB) {
            result = CreateNode(nod->value, _diff_(nod->left, var_cod), _diff_(nod->right, var_cod));
        }
        else if (nod->$value == OP_MUL) {
            Node* left = CreateNode(element{DOUBLE_OPERATOR, OP_MUL}, _diff_(nod->left, var_cod), Tree_copy(nod->right));
            Node* right = CreateNode(element{DOUBLE_OPERATOR, OP_MUL}, Tree_copy(nod->left), _diff_(nod->right, var_cod));

            result = CreateNode(element{DOUBLE_OPERATOR, OP_ADD}, left, right);
        }
        else if (nod->$value == OP_DIV) {
            Node* diff_numerator = CreateNode(element{DOUBLE_OPERATOR, OP_MUL}, _diff_(nod->left, var_cod), Tree_copy(nod->right));
            Node* diff_denominator = CreateNode(element{DOUBLE_OPERATOR, OP_MUL}, Tree_copy(nod->left), _diff_(nod->right, var_cod));

            Node* left = CreateNode(element{DOUBLE_OPERATOR, OP_SUB}, diff_numerator, diff_denominator);

            Node* right = CreateNode(element{DOUBLE_OPERATOR, OP_EXPONENT}, Tree_copy(nod->right), CreateNode(element{NUM, 2}));

            result = CreateNode(element{DOUBLE_OPERATOR, OP_DIV}, left, right);
        }
        else if (nod->$value == OP_EXPONENT) {
            bool arg_const = _is_const_(nod->left, var_cod);
            bool exp_const = _is_const_(nod->right, var_cod);

            if (arg_const && exp_const) {
                result = CreateNode(element{NUM, 0});
            }
            else if (!arg_const && exp_const) {

                Node* new_exp = CreateNode(element{DOUBLE_OPERATOR, OP_SUB}, Tree_copy(nod->right), CreateNode(element{NUM, 1}));
                new_exp = CreateNode(element{DOUBLE_OPERATOR, OP_EXPONENT}, Tree_copy(nod->left), new_exp);

                result = CreateNode(element{DOUBLE_OPERATOR, OP_MUL}, Tree_copy(nod->right),
                                    CreateNode(element{DOUBLE_OPERATOR, OP_MUL}, _diff_(nod->left, var_cod), new_exp));
            }
            else if (arg_const && !exp_const) {

                Node* ln_arg = CreateNode(element{BIN_OPERATOR, OP_LN}, (Node*)nullptr, Tree_copy(nod->left));
                Node* right = CreateNode(element{DOUBLE_OPERATOR, OP_MUL}, ln_arg, _diff_(nod->right, var_cod));

                result = CreateNode(element{DOUBLE_OPERATOR, OP_MUL}, Tree_copy(nod), right);
            }
            else {

                Node* res_left = nullptr;
                {
                    Node* new_exp = CreateNode(element{DOUBLE_OPERATOR, OP_SUB}, Tree_copy(nod->right), CreateNode(element{NUM, 1}));
                    new_exp = CreateNode(element{DOUBLE_OPERATOR, OP_EXPONENT}, Tree_copy(nod->left), new_exp);

                    res_left = CreateNode(element{DOUBLE_OPERATOR, OP_MUL}, Tree_copy(nod->right),
                                        CreateNode(element{DOUBLE_OPERATOR, OP_MUL}, _diff_(nod->left, var_cod), new_exp));
                }

                Node* res_right = nullptr;
                {
                    Node* ln_arg = CreateNode(element{BIN_OPERATOR, OP_LN}, (Node*)nullptr, Tree_copy(nod->left));
                    Node* right = CreateNode(element{DOUBLE_OPERATOR, OP_MUL}, ln_arg, _diff_(nod->right, var_cod));

                    res_right = CreateNode(element{DOUBLE_OPERATOR, OP_MUL}, Tree_copy(nod), right);
                }

                result = CreateNode(element{DOUBLE_OPERATOR, OP_ADD}, res_left, res_right);
            }
        }
        else { assert(0); }
    }
    else if (nod->$flag == BIN_OPERATOR) {

        if (nod->$value == UN_MINUS) {
            result = CreateNode(nod->value);
            result->right = _diff_(nod->right, var_cod);
        }
        else if (nod->$value == OP_LN) {
            result = CreateNode(element{DOUBLE_OPERATOR, OP_DIV}, _diff_(nod->right, var_cod), Tree_copy(nod->right));
        }
        else if (nod->$value == OP_SIN) {
            Node* left = CreateNode(element{BIN_OPERATOR, OP_COS}, (Node*)nullptr, Tree_copy(nod->right));

            result = CreateNode(element{DOUBLE_OPERATOR, OP_MUL}, left, _diff_(nod->right, var_cod));
        }
        else if (nod->$value == OP_COS) {

        }
    }

    return result;
}

bool Expression::_is_const_(Node* nod, int not_const_var) {
    if (nod == nullptr) {
        return true;
    }

    bool is_const = true;
    if (nod->$flag == VAR && nod->$value == not_const_var) {
        is_const = false;
    }

    return is_const && _is_const_(nod->left, not_const_var) && _is_const_(nod->right, not_const_var);
}







/*

void Expression::differentiation(const std::string& var_name) {
    if (root_orig == nullptr) {
        return ;
    }
    auto it = std::find(var_data.begin(), var_data.end(), var_name);
    if (it == var_data.end()) {
        throw std::invalid_argument("Unknown variable: " + var_name);
    }

    Node* tmp = _diff_(root_orig, it - var_data.begin());

    delete root_orig;
    root_orig = tmp;
}
Node *Expression::_diff_(Node *nod, int var_cod) {
    if (nod == nullptr) {
        return nullptr;
    }

    static int _diff_var;
    if (var_cod != -1) {
        _diff_var = var_cod;
    }

    Node* res = nullptr;
    if (nod->flag == OPERATION) {
        if (nod->value == ADD || nod->value == SUB) {
            res = _diff_case_lin_(nod);
        }
        else if (nod->value == MUL) {
            res = _diff_case_mul_(nod);
        }
        else if (nod->value == DIV) {
            res = _diff_case_div_(nod);
        }
        else {assert(0);}
    }
    else if (nod->flag == NUM) {
        res = _diff_case_const(nod);
    }
    else if (nod->flag == VAR) {
        if (nod->value == _diff_var) {
            res = _diff_case_var_(nod);
        }
        else {
            res = _diff_case_const(nod);
        }
    }
    else {assert(0);}

    return res;
}
Node *Expression::_diff_case_lin_(Node *nod) {
    if (nod == nullptr) {
        return nullptr;
    }
    Node* res = CreateNode(OPERATION, nod->value);

    res->left = _diff_(nod->left);
    res->left->parent = res;
    res->right = _diff_(nod->right);
    res->right->parent = res;

    return res;
}
Node *Expression::_diff_case_mul_(Node *nod) {
    if (nod == nullptr) {
        return nullptr;
    }
    Node* res_left = CreateNode(OPERATION, MUL, _diff_(nod->left), Tree_copy(nod->right));

    Node* res_right = CreateNode(OPERATION, MUL, Tree_copy(nod->left), _diff_(nod->right));

    return CreateNode(OPERATION, ADD, res_left, res_right);
}
Node *Expression::_diff_case_div_(Node *nod) {
    if (nod == nullptr) {
        return nullptr;
    }
    Node* res_left = _diff_case_mul_(nod);
    res_left->value = SUB;

    Node* res_right = CreateNode(OPERATION, MUL, Tree_copy(nod->right), Tree_copy(nod->right));

    return CreateNode(OPERATION, DIV, res_left, res_right);
}
Node *Expression::_diff_case_const(Node *nod) {
    if (nod == nullptr) {
        return nullptr;
    }

    return CreateNode(NUM, 0);
}
Node *Expression::_diff_case_var_(Node *nod) {
    if (nod == nullptr) {
        return nullptr;
    }

    return CreateNode(NUM, 1);
}


void Expression::simplification() {
    if (root_orig == nullptr) {
        return ;
    }

    _simpler_node_(root_orig);
}
void Expression::_simpler_node_(Node *nod) {
    if (nod == nullptr) {
        return ;
    }

    _simpler_node_(nod->left);
    _simpler_node_(nod->right);

    if (nod->flag == OPERATION) {

        if (nod->value == MUL) {
            _simpler_case_mul_(nod);
        }
        else if (nod->value == DIV) {
            _simpler_case_div_(nod);
        }
        else if (nod->value == ADD) {
            _simpler_case_add_(nod);
        }
        else if (nod->value == SUB) {
            _simpler_case_sub_(nod);
        }
    }
}
void Expression::_simpler_case_mul_(Node *nod) {
    if (nod->left == nullptr || nod->right == nullptr) {
        return ;
    }

    if ((nod->left->flag == NUM && nod->left->value == 0) || (nod->right->flag == NUM && nod->right->value == 0)) {
        nod->flag = NUM;
        nod->value = 0;

        delete nod->left;
        delete nod->right;
        nod->left = nod->right = nullptr;
    }
    else if (nod->left->flag == NUM && nod->left->value == 1) {
        if (nod->parent != nullptr) {
            if (nod->parent->left == nod) {

                nod->parent->left = nod->right;
                nod->right->parent = nod->parent;
            }
            else {
                nod->parent->right = nod->right;
                nod->right->parent = nod->parent;
            }
        } else {
            root_orig = nod->right;
        }
        nod->right = nullptr;
        delete nod;
        nod = nullptr;
    }
    else if (nod->right->flag == NUM && nod->right->value == 1) {
        if (nod->parent != nullptr) {
            if (nod->parent->left == nod) {

                nod->parent->left = nod->left;
                nod->left->parent = nod->parent;
            }
            else {
                nod->parent->right = nod->left;
                nod->left->parent = nod->parent;
            }
        } else {
            root_orig = nod->left;
        }
        nod->left = nullptr;
        delete nod;
        nod = nullptr;
    }
    else if (nod->left->flag == NUM && nod->right->flag == NUM) {
        nod->flag = NUM;
        nod->value = nod->left->value * nod->right->value;

        delete nod->left;
        delete nod->right;
        nod->left = nod->right = nullptr;
    }
}
void Expression::_simpler_case_div_(Node *nod) {
    if (nod->left == nullptr || nod->right == nullptr) {
        return ;
    }

    if (nod->right->flag == NUM && nod->right->value == 1) {
        if (nod->parent != nullptr) {
            if (nod->parent->left == nod) {

                nod->parent->left = nod->left;
                nod->left->parent = nod->parent;
            }
            else {
                nod->parent->right = nod->left;
                nod->left->parent = nod->parent;
            }
        } else {
            root_orig = nod->left;
        }
        nod->left = nullptr;
        delete nod;
        nod = nullptr;
    }
}
void Expression::_simpler_case_add_(Node *nod) {
    if (nod->left == nullptr || nod->right == nullptr) {
        return ;
    }

    if (nod->left->flag == NUM && nod->right->flag == NUM) {
        nod->flag = NUM;
        nod->value = nod->left->value + nod->right->value;

        delete nod->left;
        delete nod->right;
        nod->left = nod->right = nullptr;
    }
    else if (nod->left->flag == NUM && nod->left->value == 0) {
        if (nod->parent != nullptr) {
            if (nod->parent->left == nod) {

                nod->parent->left = nod->right;
                nod->right->parent = nod->parent;
            }
            else {
                nod->parent->right = nod->right;
                nod->right->parent = nod->parent;
            }
        } else {
            root_orig = nod->right;
        }
        nod->right = nullptr;
        delete nod;
        nod = nullptr;
    }
    else if (nod->right->flag == NUM && nod->right->value == 0) {
        if (nod->parent != nullptr) {
            if (nod->parent->left == nod) {

                nod->parent->left = nod->left;
                nod->left->parent = nod->parent;
            }
            else {
                nod->parent->right = nod->left;
                nod->left->parent = nod->parent;
            }
        } else {
            root_orig = nod->left;
        }
        nod->left = nullptr;
        delete nod;
        nod = nullptr;
    }
}
void Expression::_simpler_case_sub_(Node *nod) {
    if (nod->left == nullptr || nod->right == nullptr) {
        return ;
    }

    if (nod->left->flag == NUM && nod->right->flag == NUM) {
        nod->flag = NUM;
        nod->value = nod->left->value - nod->right->value;

        delete nod->left;
        delete nod->right;
        nod->left = nod->right = nullptr;
    }
    else if (nod->left->flag == NUM && nod->left->value == 0) {
        nod->flag = OPERATION;
        nod->value = MUL;
        nod->left->value = -1;
    }
    else if (nod->right->flag == NUM && nod->right->value == 0) {
        if (nod->parent != nullptr) {
            if (nod->parent->left == nod) {

                nod->parent->left = nod->left;
                nod->left->parent = nod->parent;
            }
            else {
                nod->parent->right = nod->left;
                nod->left->parent = nod->parent;
            }
        } else {
            root_orig = nod->left;
        }
        nod->left = nullptr;
        delete nod;
        nod = nullptr;
    }
}
*/

