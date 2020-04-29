#include <iostream>

#include "Differentiator.h"

#include <fstream>
#include <ctime>

int main() {

    Expression exp1("-x");
    Expression exp2("6 *(-3 / x)");
    Expression exp3("((-x + 5) + 3)");
    Expression empty;



    exp3.OutExpression(std::cout);

    //std::stringstream in("((((7 + 3) ^ 3 / ((y + 5) * (76 - 4))^90)^4 + 5) * (ch((arcsin(tg(1/x))) + 89) / (x + (ctg(x + 8)))^5))^(ctg(sin(sqrt(y/x))))");
    /*std::stringstream in("((ln(x))^(x) / (sin(ln(x))))^(x)");
    Expression test;


    test.ReadExpression(in);

    test.OutExpression(std::cout);
    Expression test2 = test.differentiation("x");*/

    std::ofstream file("tx1.tex");
    file   << "\\documentclass[12pt]{article}\n"
              "\\usepackage[russian]{babel}\n"
              "\\begin{document}\n"
              "{\n"
              "\\[ \n";

    exp3.OutExpressionTeX(file);

    file   << "\n\\]\n"
              "}\n"
              "\\end{document}";

    std::cout << std::endl;
    //test2.OutExpression(std::cout);

    //std::cout << std::endl << test.calculate({{"x", 10}, {"y", 5}});
    //std::cout << std::endl << test.calculate();
/*
    test.clear();

    test.simplification();

    test.OutExpression(std::cout);
    std::cout << std::endl;

    test.differentiation("x");

    test.simplification();
    test.OutExpression(std::cout);
    std::cout << std::endl;*/




    return 0;
}
