#include "expression.h"

#include <cstring>
#include <algorithm>
#include <cassert>
#include <deque>

#define Node Node<element>
#define $lp left->parent
#define $rp right->parent
#define $flag value.flag
#define $value value.value

Expression::Expression()
: root_orig(nullptr) {
}

Expression::Expression(std::istream &stream)
: root_orig(nullptr) {
    ReadExpression(stream);
}


void Expression::ReadExpression(std::istream &stream) {
    delete root_orig;

    std::vector<element> exp = _preprocessor_for_read_(stream);
    root_orig = _read_(exp);
}
std::vector<element> Expression::_preprocessor_for_read_(std::istream &stream) {
    std::vector<element> result;

    char current_sign = stream.peek();
    while (stream) {
        if (current_sign == ' ' || current_sign == '\t' || current_sign == '\n') {
            /*nop()*/;
            stream.ignore(1);
        }
        else if (current_sign == '(' || current_sign == ')') {
            result.push_back( {NOT_OPERATOR, static_cast<double>(current_sign)} );
            stream.ignore(1);
        } else if (_in_(current_sign, "+-*/^")) {
            result.push_back({DOUBLE_OPERATOR, static_cast<double>(current_sign)});
            stream.ignore(1);
        }
        else {
            std::string str_elem = _get_str_(stream, " +-*/^()[]\t\n");

            if (!str_elem.empty()) {
                element cod_elem = _coder_operator_(str_elem);
                if (cod_elem.flag == NOT_OPERATOR) {

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
    auto it = oper_to_cod.find(_str);

    if (it == oper_to_cod.end()) {
        return { NOT_OPERATOR, 0};
    }
    if (it->second >= BIN_OPERATOR) { //номер бинарной операции начинается со 100
        return {BIN_OPERATOR, static_cast<double>(it->second)};
    }
    return {DOUBLE_OPERATOR, static_cast<double>(it->second)};
}


Node *Expression::_read_(std::vector<element>& _str) {
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
    else if (_str[0].flag == NOT_OPERATOR && _str[0].value == OPEN_BRACKET) {
        result = _read_brackets_case_(it_begin, it_end);
    }
    else {
        throw std::invalid_argument("non-violent record of expression");
    }
    return result;
}
Node* Expression::_read_num_or_var_case_(std::vector<element>::iterator it_begin, std::vector<element>::iterator it_end) {
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

            result->left = num;
            num = nullptr;
        }
        else if (new_begin->value == OP_MUL || new_begin->value == OP_DIV) {
            result = _read_mul_or_div_case_(new_begin, it_end, num);
            assert(result != nullptr);

            num = nullptr;
        }
        /*else if (new_begin->value == OP_EXPONENT) {
//            result = _read_exponent_case_(new_begin, it_end);
            assert(result != nullptr);

            if (result->$flag == DOUBLE_OPERATOR && result->$value == OP_EXPONENT) {

            } else if (result->left->$flag == DOUBLE_OPERATOR && result->left->$value == OP_EXPONENT) {

            } else { assert(0); }
        }*/
        else { assert(0); }
    }
    else if (new_begin->flag == NOT_OPERATOR && new_begin->value == CLOSE_BRACKET) {
        result = num;
        num = nullptr;
    }
    else { throw std::runtime_error("Expected 'DOUBLE_OPERATOR' or 'CLOSE_BRACKET' but received: " + std::to_string(new_begin->flag)); }

    return result;
}
Node* Expression::_read_add_or_sub_case_(std::vector<element>::iterator it_begin, std::vector<element>::iterator it_end) {
    Node* result = CreateNode(*it_begin);
    Node* num = nullptr;

    auto new_begin = it_begin;
    if (it_begin != it_end) {
        new_begin++;
    } else {
        throw std::runtime_error("Expression ends with Double operator.");
    }

    if (new_begin->flag == NUM || new_begin->flag == VAR) {
        result->right = _read_num_or_var_case_(new_begin, it_end);
        assert(result->right != nullptr);
    }
    else if (new_begin->flag == NOT_OPERATOR && new_begin->value == OPEN_BRACKET) {
        result->right = _read_brackets_case_(new_begin, it_end);
        assert(result->right != nullptr);
    }
    else if (new_begin->flag == BIN_OPERATOR) {
        result->right = _read_binary_case_(new_begin, it_end);
        assert(result->right != nullptr);
    }
    else { throw std::runtime_error("in _read_add_or_sub_case_, received: " + std::to_string(new_begin->flag)); }

    return result;
}
Node* Expression::_read_mul_or_div_case_(std::vector<element>::iterator it_begin, std::vector<element>::iterator it_end, Node* _left) {
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
            result->right = tmp->left;
            tmp->left = result;
            result = tmp;
        }
        else {
            result->right = tmp;
        }
    }
    else if (new_begin->flag == NOT_OPERATOR && new_begin->value == OPEN_BRACKET) {
        auto new_end = _find_closing_bracket_(new_begin, it_end + 1);
        assert(new_end != it_end + 1);

        result->right = _read_brackets_case_(new_begin, new_end);
        assert(result->right != nullptr);

        //после скобок
        if (new_end != it_end) {
            new_begin = new_end + 1;

            if (new_begin->flag == DOUBLE_OPERATOR) {
                if (new_begin->value == OP_ADD || new_begin->value == OP_SUB) {
                    Node* tmp = _read_add_or_sub_case_(new_begin, it_end);
                    assert(tmp != nullptr);

                    tmp->left = result;
                    result = tmp;
                    tmp = nullptr;
                }
                else if (new_begin->value == OP_MUL || new_begin->value == OP_DIV) {
                    result = _read_mul_or_div_case_(new_begin, it_end, result);
                    assert(result != nullptr);

                } else { throw std::runtime_error("in bracket case(after) value:" + std::to_string(new_begin->value)); }
            }else { throw std::runtime_error("in bracket case(after) flag:" + std::to_string(new_begin->flag)); }
        }
    }
    else if (new_begin->flag == BIN_OPERATOR) {
        Node* tmp = _read_binary_case_(new_begin, it_end);
        assert(tmp != nullptr);

        if (tmp->$flag == DOUBLE_OPERATOR && (tmp->$value == OP_ADD || tmp->$value == OP_SUB)) {
            result->right = tmp->left;
            tmp->left = result;
            result = tmp;
        }
        else {
            result->right = tmp;
        }
    }
    else { throw std::runtime_error("in _read_add_or_sub_case_, received: " + std::to_string(new_begin->flag)); }

    return result;
}
Node* Expression::_read_brackets_case_(std::vector<element>::iterator it_begin, std::vector<element>::iterator it_end) {
    auto new_begin = it_begin;
    if (it_begin != it_end) {
        new_begin++;
    } else {
        throw std::runtime_error("Expression ends with open bracket.");
    }

    std::vector<element>::iterator end_bracket = _find_closing_bracket_(it_begin, it_end + 1);

    if (end_bracket == it_end + 1 /*&& end_bracket->flag != NOT_OPERATOR && end_bracket->value != CLOSE_BRACKET*/) {
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
    else if (new_begin->flag == NOT_OPERATOR && new_begin->value == OPEN_BRACKET) {
        inside_brackets = _read_brackets_case_(new_begin, end_bracket);
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
            } else { throw std::runtime_error("in bracket case(after) value:" + std::to_string(new_begin->value)); }
        }else { throw std::runtime_error("in bracket case(after) flag:" + std::to_string(new_begin->flag)); }
    }
    return result;
}
Node* Expression::_read_binary_case_(std::vector<element>::iterator it_begin, std::vector<element>::iterator it_end) {
    Node* bin_operator = CreateNode(*it_begin);

    std::vector<element>::iterator new_begin;
    if (it_begin == it_end) {
        throw std::runtime_error("Expression ends with binary operator(without argument).");
    } else {
        new_begin = it_begin + 1;
    }

    //argument
    std::vector<element>::iterator end_bracket;
    if (new_begin->flag == NOT_OPERATOR && new_begin->value == OPEN_BRACKET) {

        end_bracket = _find_closing_bracket_(new_begin, it_end);
        if (end_bracket == it_end && end_bracket->flag != NOT_OPERATOR && end_bracket->value != CLOSE_BRACKET) {
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

void Expression::clear() {
    delete root_orig;
    root_orig = nullptr;

    var_data.clear();
}



void Expression::OutExpressionTree(std::ostream &_stream) const {
    LNR_for_output_tree(root_orig, _stream);
}
void Expression::LNR_for_output_tree(Node *nod, std::ostream &stream) const {
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
    LNR_for_output_without_br(root_orig, stream);
}
void Expression::LNR_for_output_without_br(Node *nod, std::ostream &stream) const {
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

    _for_output_TeX_(root_orig, stream);
}
void Expression::_for_output_TeX_(Node* nod, std::ostream &stream) const {
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

            if (nod->left->$flag == DOUBLE_OPERATOR && oper_to_priority.at(nod->$value) > oper_to_priority.at(nod->left->$value)) {
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
        else {
            stream << "\\" << cod_to_oper.at(nod->$value) << "\\left(";
            _for_output_TeX_(nod->right, stream);
            stream << "\\right)";
        }
    } else { assert(0); }


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

std::vector<element>::iterator
Expression::_find_closing_bracket_(std::vector<element>::iterator it_begin, std::vector<element>::iterator it_end) {
    std::vector<element>::iterator end_bracket;

    int count_brackets = 0; //считая первую скобку
    end_bracket = std::find_if(it_begin, it_end, [&count_brackets](const element &x) {
        if (x.flag == NOT_OPERATOR && x.value == OPEN_BRACKET) {
            count_brackets++;
        }
        if (x.flag == NOT_OPERATOR && x.value == CLOSE_BRACKET) {
            count_brackets--;
        }
        return count_brackets == 0;
    });

    return end_bracket;
}
