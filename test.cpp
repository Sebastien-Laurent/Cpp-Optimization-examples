#include <memory>
#include <iostream>

void boosterScore(std::unique_ptr<int> s) {
    *s += 1000; // On modifie directement, syntaxe ultra simple
    std::cout << *s;
}

int main() {
    auto score = std::make_unique<int>(100);
    boosterScore(std::move(score)); // On déréférence pour passer la valeur par référence

}