
/*
    Уверен, тема с войдовыми функциями и ассертами внутри - это
    идея Деда. Если да, то он наверняка должен был сказать, что
    это не оч и надо сделать по-другому. Вообще войдовые функции
    лучше стараться не юзать, они должны ведь сигнализировать об 
    ошибках, и выходить, посылая проге SIGABORT (assert, кажется,
    так и делает) - не оч тема
 */


#include<iostream>
#include<stdio.h>
#include<stdlib.h>
#include<assert.h>

#define START_MEM_SIZE    8
#define LOG_FILE          "results.txt"
#define CANARY_CONST      0xBEDAEDAA

/*#define UNITTEST(what, op, ref)\
{\
    int result = (what)\
    if(result op (ret))\
        printf(#what "[passed]");\
    else\
        printf(#what " is %d, but " #ref  "is %d in line %d", result, ())
*/

typedef int elem_type;

typedef struct Stack
{
    unsigned int start_canary;
    unsigned int size_stack;
    unsigned int memory_size;
    elem_type* array_stack;
    elem_type control_sum;
    unsigned int end_canary;
}Stack;

/*!
    Function for verification stack
*/

bool StackOk(const Stack* stk)
{
/*
    stk может быть нулевым, тогда упадет :(
 */
    elem_type sum = 0;
    for(unsigned int i = 0; i < stk->size_stack; i++)
    {
        sum += stk->array_stack[i];
    }
    return stk && (0 <= stk->size_stack &&
                   stk->size_stack <= stk->memory_size) &&
                   stk->array_stack &&
                   stk->start_canary == CANARY_CONST &&
                   stk->end_canary == CANARY_CONST &&
                   stk->control_sum == sum;
}

/*!
    Function for writing info about stack in file
*/

void StackDump(const Stack* stk, const char* err_info)
{
    FILE* log = fopen(LOG_FILE, "a");
/*
    Проверочку бы сюда...
 */

    fprintf(log, "Stack \"stk\": 0x%p\n", stk);
    fprintf(log, "%s\n", err_info);
    fprintf(log, "size = %d, memory = %d, array = 0x%p\n",
             stk->size_stack, stk->memory_size, stk->array_stack);
    printf("%d", stk->size_stack);
    for (unsigned int i = 0; i < stk->size_stack; i++)
    {
        fprintf(log, "array[%d] = %d\n", i, stk->array_stack[i]);
    }
    fclose(log);
}


/*!
    Function for creating elements of stack

    @param stack_test - stack for initialization
*/

/*
    Функция может завершиться неуспешно, не возвращать ли ошибку?
 */

void stack_init(Stack* stack_test)
{
    stack_test->start_canary = CANARY_CONST;
    stack_test->size_stack = 0;
    stack_test->memory_size = START_MEM_SIZE;
    stack_test->array_stack = (
                elem_type*)calloc(stack_test->memory_size, sizeof(elem_type));
/*
    Проверку calloc'а
 */
    stack_test->control_sum = 0;
    stack_test->end_canary = CANARY_CONST;
    //assert(stack_test->array_stack);

    if(!StackOk(stack_test))
    {
        StackDump(stack_test, "ERROR in end of stack_init");
        assert(!"stack_init");
    }
}

/*!
    Function for pushing element in stack

    @param stack_test - stack for pushing
    @param pushing_elem - element, which need pushing
*/

/*
    Функция тоже может обломаться, вернуть тогда ошибку лучше
 */
void push(Stack* our_stack, elem_type pushing_elem)
{
    if(!StackOk(our_stack))
    {
        StackDump(our_stack, "ERROR in start of push");
        assert(!"push");
    }

    if(our_stack->size_stack > our_stack->memory_size)
    {
/*
    Одну строку не надо брать в скобки фигурные
 */
        std::cout << "ERROR with size of stack and memory";
    }

    if(our_stack->size_stack == our_stack->memory_size)
    {
        our_stack->memory_size *= 2;
        our_stack->array_stack = (elem_type*)realloc(
                            our_stack->array_stack,
                            our_stack->memory_size * sizeof(elem_type));
/*
    Не надо делать проверки ассертами. Ифами лучше.
 */
        assert(our_stack->array_stack);
    }
    our_stack->size_stack++;
    our_stack->array_stack[our_stack->size_stack - 1] = pushing_elem;
    our_stack->control_sum += pushing_elem;

    if(!StackOk(our_stack))
    {
        StackDump(our_stack, "ERROR in end of push");
        assert(!"push");
    }
}

/*!
    Function for taking element from stack

    @param stack_test - stack from taking

    @return Last element of stack
*/

elem_type pop(Stack* our_stack)
{
    if(!StackOk(our_stack))
    {
        StackDump(our_stack, "ERROR in start of pop");
        assert(!"pop");
    }

    //assert(our_stack->array_stack[our_stack->size_stack-1]);
    elem_type result = our_stack->
        array_stack[our_stack->size_stack-1];
    our_stack->size_stack--;
    return result;
/*
    Зачем после ретерна проверки? Они ж никогда не выполнятся
 */
    if(!StackOk(our_stack))
    {
        StackDump(our_stack, "ERROR in end of pop");
        assert(!"pop");
    }
}

/*!
    Function for deleting elements of stack

    @param stack_test - stack for deleting
*/

void delete_stack(Stack* our_stack)
{
    if(!StackOk(our_stack))
    {
        StackDump(our_stack, "ERROR in start of delete_stack");
        assert(!"delete_stack");
    }

    free(our_stack->array_stack);

/*
    Указатель же. Не круто приравнивать указатель нулю, особенно в С++
 */
    our_stack->size_stack = 0;
}

int main()
{
    Stack stack_test;
    stack_init(&stack_test);
    for(int i = 0; i < 10; i++)
    {
        push(&stack_test, i);
        std::cout << stack_test.array_stack[i];

    }
    for(int i = 0; i < 10; i++)
    {
        printf("pop %d\n", pop(&stack_test));
    }

    delete_stack(&stack_test);
    return 0;
}
