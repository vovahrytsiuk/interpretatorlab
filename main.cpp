#include "source.h"

int main()
{
	cout << "Enter file with code: ";
	string filename; cin >> filename;

	const ast new_ast(filename);
	/*const ast new_ast("expression.txt");*/
	new_ast.calculate();

	return 0;
}
