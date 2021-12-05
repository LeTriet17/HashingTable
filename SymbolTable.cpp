#include "SymbolTable.h"
HashTable *seleted_hash(string line)
{
    std::regex num("\\d+");
    sregex_iterator it = sregex_iterator(line.begin(), line.end(), num);
    int *arg = new int[distance(it, sregex_iterator())];
    for (sregex_iterator i = it; i != sregex_iterator(); ++i)
        arg[distance(it, i)] = stoi(i->str());
    if (regex_match(line, regex("LINEAR \\d+ \\d+")))
        return new LiHashTable(arg[0], arg[1]);
    else if (regex_match(line, regex("QUADRATIC \\d+ \\d+ \\d+")))
        return new QuadHashTable(arg[0], arg[1], arg[2]);
    else if (regex_match(line, regex("DOUBLE \\d+ \\d+")))
        return new DouHashTable(arg[0], arg[1]);
    else
        throw InvalidInstruction(line);
}
int check_func(Slot *func_node, string line, HashTable *hash_table, string func_name)
{
    int prob = 0;
    if (func_node && !func_node->is_func())
        throw TypeMismatch(line);
    else if (!func_node)
        throw Undeclared(func_name);
    string param(&line[line.find_first_of("(") + 1], &line[line.find_last_of(")")]);
    regex func_regex("([0-9]+|'(\\w|\\s)*'|[a-z]\\w*)");
    Slot::Type *curr = func_node->param;
    if (!curr || (std::distance(
                      std::sregex_iterator(param.begin(), param.end(), func_regex),
                      std::sregex_iterator()) != func_node->size_param))
        throw TypeMismatch(line);
    prob += func_node->probing;
    for (sregex_iterator it = sregex_iterator(param.begin(), param.end(), func_regex); it != sregex_iterator(); it++, curr++)
    {

        if (regex_match(it->str(), regex("[a-z]\\w*")))
        {
            string name = it->str();
            Slot *node = hash_table->find(name);
            if (node)
            {
                if (*curr == Slot::UNKNOWN)
                {
                    if (node->type == Slot::UNKNOWN)
                        throw TypeCannotBeInferred(line);
                    else
                        *curr = node->type;
                    prob += node->probing;
                }
                else if (node->type == Slot::UNKNOWN)
                    node->type = *curr;
                else if (node->type != *curr)
                    throw TypeMismatch(line);
            }
            else
                throw Undeclared(name);
            continue;
        }
        else if (regex_match(it->str(), regex("'(\\w|\\s)*'")) && *curr == Slot::UNKNOWN)
        {
            *curr = Slot::STRING;
            continue;
        }
        else if (regex_match(it->str(), regex("[0-9]+")) && *curr == Slot::UNKNOWN)
        {
            *curr = Slot::NUMBER;
            continue;
        }
        else if ((regex_match(it->str(), regex("'(\\w|\\s)*'")) && *curr == Slot::NUMBER) || (regex_match(it->str(), regex("[0-9]+")) && *curr == Slot::STRING))
            throw TypeMismatch(line);
    }
    return prob;
}
void SymbolTable::run(string filename)
{
    ifstream infile(filename);
    string line;
    getline(infile, line);
    HashTable *hash_table = seleted_hash(line);
    regex iden_name("[a-z]\\w*");
    regex num("[0-9]+");
    while (getline(infile, line))
    {
        if (regex_match(line, regex("INSERT [a-z]\\w*(\\s[0-9]+)?")))
        {
            if (hash_table->isFull())
                throw Overflow(line);
            Slot slot;
            string name = sregex_iterator(line.begin(), line.end(), iden_name)->str();
            sregex_iterator it = sregex_iterator(line.begin() + 7 + name.length(), line.end(), num);
            if (it != sregex_iterator())
            {
                if (block)
                    throw InvalidDeclaration(name);
                int value = stoi(it->str());
                slot = Slot(name, block, value);
            }
            else
                slot = Slot(name, block);
            Slot *tmp = hash_table->find(name);
            if (tmp && *tmp == slot)
                throw Redeclared(tmp->name);
            if (!hash_table->insert(slot))
                throw Overflow(line);
        }
        else if (regex_match(line, regex("ASSIGN [a-z]\\w* [0-9]+")))
        {
            string name = sregex_iterator(line.begin(), line.end(), iden_name)->str();
            Slot *var = hash_table->find(name);
            if (var)
            {
                if (var->type == Slot::UNKNOWN)
                    var->type = Slot::NUMBER;
                else if (var->type != Slot::NUMBER)
                    throw TypeMismatch(line);
                cout << var->probing << endl;
            }
            else
                throw Undeclared(var->name);
        }
        else if (regex_match(line, regex("ASSIGN [a-z]\\w* '(\\w|\\s)*'")))
        {
            string name = sregex_iterator(line.begin(), line.end(), iden_name)->str();
            Slot *var = hash_table->find(name);
            if (var)
            {
                if (var->type == Slot::UNKNOWN)
                    var->type = Slot::STRING;
                else if (var->type != Slot::STRING)
                    throw TypeMismatch(line);
                cout << var->probing << endl;
            }
            else
                throw Undeclared(var->name);
        }
        else if (regex_match(line, regex("ASSIGN [a-z]\\w* [a-z]\\w*")))
        {
            regex name_regex(("[a-z]\\w*"));
            string name[2];
            int i = 0;
            for (sregex_iterator it = sregex_iterator(line.begin(), line.end(), name_regex); it != sregex_iterator(); it++, i++)
                name[i] = it->str();
            Slot *var1 = hash_table->find(name[1]);
            if (var1)
            {
                Slot *var0 = hash_table->find(name[0]);
                if (var0)
                {
                    if (var0->type != Slot::UNKNOWN && var1->type != Slot::UNKNOWN)
                    {
                        if (var0->type != var1->type)
                            throw TypeMismatch(line);
                        cout << var0->probing + var1->probing << endl;
                    }
                    else if (var0->type == Slot::UNKNOWN && var1->type == Slot::UNKNOWN)
                        throw TypeCannotBeInferred(line);
                    else if (var0->type == Slot::UNKNOWN)
                    {
                        var0->type = var1->type;
                        cout << var1->probing << endl;
                    }
                    else
                    {
                        var1->type = var0->type;
                        cout << var0->probing << endl;
                    }
                }
                else
                    throw Undeclared(name[0]);
            }
            else
                throw Undeclared(name[1]);
        }
        else if (regex_match(line, regex("ASSIGN [a-z]\\w* [a-z]\\w*\\((([0-9]+|'(\\w|\\s)*'|[a-z]\\w*)(,[0-9]+|,'(\\w|\\s)*'|,[a-z]\\w*)*)?\\)")))
        {
            string name_func(&line[line.find_first_not_of("ASSIGN ")], &line[line.find_first_of("(")]);
            string iden_name(&name_func[0], &name_func[name_func.find_first_of(" ")]);
            string func_name(&name_func[iden_name.length()] + 1, &name_func[name_func.length()]);
            Slot *func_node = hash_table->find(func_name);
            int prob = check_func(func_node, line, hash_table, func_name);
            Slot *iden_node = hash_table->find(iden_name);
            if (iden_node)
            {
                if (iden_node->type == Slot::UNKNOWN && func_node->type == Slot::UNKNOWN)
                    throw TypeCannotBeInferred(line);
                if (iden_node->type == Slot::UNKNOWN)
                    iden_node->type = func_node->type;
                if (func_node->type == Slot::UNKNOWN)
                    func_node->type = iden_node->type;
                else if (iden_node->type != func_node->type)
                    throw TypeMismatch(line);
                prob += iden_node->probing;
            }
            else
                throw Undeclared(iden_name);
            cout << prob << endl;
        }
        else if (regex_match(line, regex("CALL [a-z]\\w*\\((([0-9]+|'(\\w|\\s)*'|[a-z]\\w*)(,[0-9]+|,'(\\w|\\s)*'|,[a-z]\\w*)*)?\\)")))
        {
            string func_name(&line[line.find_first_not_of("CALL ")], &line[line.find_first_of("(")]);
            Slot *func_node = hash_table->find(func_name);
            check_func(func_node, line, hash_table, func_name);
            if (func_node->type == Slot::UNKNOWN)
                func_node->type = Slot::VOID;
            else if (func_node->type != Slot::VOID)
                throw TypeMismatch(line);
            cout << func_node->probing << endl;
        }
        else if (regex_match(line, regex("BEGIN")))
        {
            block++;
        }
        else if (regex_match(line, regex("END")))
        {
            if (block < 1)
                throw UnknownBlock();
            hash_table->remove(block--);
        }
        else if (regex_match(line, regex("LOOKUP [a-z]\\w*")))
        {
            string name = sregex_iterator(line.begin(), line.end(), iden_name)->str();
            Slot *var = hash_table->find(name);
            if (var)
                cout << var->index << endl;
            else
                throw Undeclared(name);
        }
        else if (regex_match(line, regex("PRINT")))
            hash_table->print();
        else
            throw InvalidInstruction(line);
    }
    if (block > 0)
        throw UnclosedBlock(block);
    infile.close();
}
