#include <iostream>
#include "expression.h"

#include <fstream>

struct elem {
    uint8_t flag;
    int value;
};

int main() {

    std::stringstream in("((7 + 3) / ((y + 5) * (76 - 4)) + 5) * ch((sin(tg(1/x)) + 89) / (x + ctg(x + 8)))");

    Expression test;

    test.ReadExpression(in);
    test.OutExpression(std::cout);

    std::ofstream file("tx1.tex");
    file   << "\\documentclass[12pt]{article}\n"
              "\\usepackage[russian]{babel}\n"
              "\\begin{document}\n"
              "{\n"
              "$$ ";

    test.OutExpressionTeX(file);

    file   << " $$\n"
              "}\n"
              "\\end{document}";
/*
    test.simplification();

    test.OutExpression(std::cout);
    std::cout << std::endl;

    test.differentiation("x");

    test.simplification();
    test.OutExpression(std::cout);
    std::cout << std::endl;*/




    return 0;
}
