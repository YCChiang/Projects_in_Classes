# Construct a push-down automaton

## Homework

### Purpose
1. Understand the concept of push-down automata.

2. Master the operation process of push-down automata, and master the use of stack in the process of state transition.

### Content

$\Sigma={a,b,c} L={w|w\in{a,b,c}*,\#_b(w)=\#_c(w)}$, $\#_z(w)$ is the number of times the character z appears in w.

## Main idea

![](D:\Code\Projects_in_Class\Theory_of_Computation\PDA\main_idea_complex.png)

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
