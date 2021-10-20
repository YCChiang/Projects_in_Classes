# Construct a push-down automaton

## Homework

### Purpose
1. Understand the concept of push-down automata.

2. Master the operation process of push-down automata, and master the use of stack in the process of state transition.

### Content

$\Sigma={a,b,c} L={w|w\in{a,b,c}*,\#_b(w)=\#_c(w)}$, $\#_z(w)$ is the number of times the character z appears in w.

## Main idea

![](D:\Code\Projects_in_Class\Theory_of_Computation\PDA\main_idea.png)

## Shift Function for PDA

![](D:\Code\Projects_in_Class\Theory_of_Computation\PDA\shift_function.png)

## Usage

Compile files:
```
gcc -o pda PDA.c
```

Usage:

```
Usage: nfa.exe [options]
General Options:
    -h, --help      Show help
    -s, string      The string to be recognized.
```

## Format Example for PDA Configuration file

state_number:
4
states:
q1,q2,q3,q4,
start_state:
q1
accept_state_number:
1
accept_state(s):
q2,
alphabet_number:
3
alphabets:
a,b,c,
shift_function:
current_state,next_state,input_alphabet,out_stack,in_stack
q1,q2,epsilon,epsilon,\$,
q2,q2,a,epsilon,epsilon,
q2,q2,epsilon,\$,epsilon,
q2,q3,b,epsilon,b,
q2,q4,c,epsilon,c,
q3,q3,a,epsilon,epsilon,
q3,q3,b,epsilon,b,
q3,q3,c,b,epsilon,
q4,q4,a,epsilon,epsilon,
q4,q4,b,c,epsilon,
q4,q4,c,epsilon,c,
q3,q2,epsilon,\$,\$,
q4,q2,epsilon,\$,\$,
