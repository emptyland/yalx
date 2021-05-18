# AST and IL Nodes

## Node Tree

```
Node
    +: il
    +- FileUnit
    +- Statement
        +- Block
        +- BreakStatement
        +- ContinueStatement
        +- ReturnStatement
        +- WhileStatement
        +- ForEachStatement
        +- ForStepStatement
        +- Declaration
            +- VariableDeclaration
            +- FunctionDeclaration
            +- ObjectDeclaration
        +- Definition 
            +- InterfaceDefinition
            +- ClassDefinition
            +- AnnotationDefinition
        +- Expression
            +- Identifier
            +- Literal
                +- UnitLiteral
                +- ActualLiteral<T>
            +- ExpressionWithOperands<N>
                +- BinaryExpression
                    +- Add
                    +- Sub
                    +- Mul
                    +- Div
                    +- Mod
                    +- Equal
                    +- NotEqual
                    +- Less
                    +- LessEqual
                    +- Greater
                    +- GreaterEqual
                    +- And
                    +- Or
                    +- BitwiseAnd
                    +- BitwiseOr
                    +- BitwiseXor
                    +- ChannelWrite
                    +- IndexedGet
                +- UnaryExpression
                    +- Negative
                    +- Not
                    +- BitwiseNegative
                    +- ChannelRead
            +- Dot
            +- Casting
            +- Calling
            +- IfExpression
            +- WhenExpression
            +- ArrayInitializer
            +- ArrayDimension
    +- Type
        +- ArrayType
        +- ClassType
        +- InterfaceType
        +- FunctionPrototype
```


## Base Node

* Class Node is arean object.
* Only allocate from arean.
* All AST/IL nodes base class

```
class Node
    = Accept(NodeVisitor*): Visitation // Accept visitor
    = IsXXX(): bool // Type checker
    ...
    = AsXXX(): XXX* // Type casting
    ...
    + kind: Kind // Kind of node
    + source_position: SourcePosition // Source file position
```

```
class SourcePosition
    + begin_column: int
    + begin_line: int
    + end_column: int
    + end_line: int
```

## AST Nodes

```
class FileUnit : Node
    + file_name: NString *
    + file_full_path: NString *
    + package_name: NString *
    + imports: List<ImportEntry> *
    + statements: List<Statement *> *
    + definitions: List<Definition *> *
    + declarations: List<Declaration *> *
```

```
class ImportEntry : Node
    + original_package_name: NString *
    + package_path: NString *
    + alias: NString *
```

```
class Declaration : Statement
    = Identifier(): Identifer *
    = Type(): TypeRef *
    = Item(int i): Declaration *
    = ItemSize(): size_t
    + annotations: List<Annotation *>
    + access: {kExport, kPublic, kProtected, kPrivate, kDefault}
```

```
class VariableDeclaration : Declaration
    + constraint: {kVal, kVar}
    + is_volatile: bool
    + variables: List<Item *>
    class Item : Declaration
        + identifer: Identifer *
        + type: Type *
    + initilaizers: List<Expression *>
```

```
class FunctionDeclaration : Declaration
    + name: NString *
    + decoration: {kNative, kAbstract, kOverride}
    + define_for: Definition *
    + prototype: FunctionPrototype *
    + reduece: Expression * // 'fun foo() ->' syntax
    + body: Block * // 'fun foo(): xxx {...}' syntax
```

```
class ObjectDeclaration : Declaration
    + name: NString *
    + methods: List<FunctionDeclaration *>
    + members: List<VariableDeclaration *>
```

```
class Definition : Statement
    + annotations: List<Annotation *>
    + generic_params: List<GenericParameter *>
    + has_instantiated: bool
```

```
class GenericParameter : Node
    = Identifier(): Symbol *
    + identifier: Type *
    + constraint: Type *
```

```
class InterfaceDefinition : Definition
    + name: NString *
    + methods: List<FunctionDeclaration *>
```

```
class ClassDefinition : Definition
    + name: NString *
    + constructor: List<VariableDeclaration *> // class Foo(val i: int, j: int) {}
    + base: Type *
    + super_calling: List<Expression *>
    + methods: List<FunctionDeclaration *>
    + members: List<VariableDeclaration *>
```

```
class Symbol : Node
    + prefix_name: NString *
    + name: NString *
```

### Annotation

```
class AnnotationDefinition : Definition
    + name: NString *
    + members: List<VariableDeclaration *>
```

```
class Annotation : Node
    + name: Symbol *
    class Field : Node
        + name: NString *
        + value: Expression *
    + fields: List<Field *>
```

### Statements

```
class Statement : Node
```

```
class Block : Statement
    + statements: List<Statement *>
```

```
class BreakStatement : Statement
```

```
class ContinueStatement : Statement
```

```
class ReturnStatement : Statement
    + values: List<Expression *>
```

```
class Assignment : Statement
    + lvals: List<Expression *>
    + rvals: List<Expression *>
```

```
class WhileStatement : Statement
    + condition: Expression *
    + body: Block *

// while (foo()>0) {
//    println("ok")
// }
```

```
class ForEachStatement : Statement
    + value: VariableDeclaration *
    + container: Expression *
```

```
class ForStepStatement : Statement
    + value: VariableDeclaration *
    + begin: Expression *
    + end: Expression *
    + bound: {kUntil, kTo}
```

### Expression

```
class Expression : Statement
    = is_lval(): bool
    = is_rval(): bool
    = is_only_lval(): bool
    = is_only_rval(): bool
    + is_lval: bool
    + is_rval: bool
```

```
class Identifer(rval = true, lval = true) : Expression
    + name: NString *
```

```
class Literal(rval = true, lval = false) : Expression
    = Type(type: Type *): Type *
    + type: Type *
```

```
class UnitLiteral : Literal
```

```
class ActualLiteral<T> : Literal
    + value: T

IntLiteral  <- ActualLiteral<int>
UIntLiteral <- ActualLiteral<unsigned>
I64Literal  <- ActualLiteral<int64_t>
U64Literal  <- ActualLiteral<uint64_t>
BoolLiteral <- ActualLiteral<bool>
StringLiteral <- ActualLiteral<NString *>
```

```
class ArrayInitializer(rval = true, lval = false) : Expression
    = dimension_count(): int
    + hint_type: ArrayType *
    + dimension: List<ArrayDimension *>

class ArrayDimension(rval = true, lval = false) : Expression
    + values: List<ArrayDimension *| Expression *>
```

```
class ExpressionWithOperands<N>(rval = true, lval = false) : Expression
    = lhs(): Expression *
    = rhs(): Expression *
    = operand(i: int): Expression *
    + operand_count: int
    + operands: Expression *[N]

BinaryExpression <- ExpressionWithOperands<2>
UnaryExpression <- ExpressionWithOperands<1>

Negative <- UnaryExpression
Add <- BinaryExpression
Sub <- BinaryExpression
Mul <- BinaryExpression
Div <- BinaryExpression
Mod <- BinaryExpression

Equal <- BinaryExpression
NotEqual <- BinaryExpression
Less <- BinaryExpression
LessEqual <- BinaryExpression
Greater <- BinaryExpression
GreaterEqual <- BinaryExpression

And <- BinaryExpression
Or <- BinaryExpression
Not <- UnaryExpression

BitwiseAnd <- BinaryExpression
BitwiseOr <- BinaryExpression
BitwiseXor <- BinaryExpression
BitwiseNegative <- UnaryExpression

ChannelRead <- UnaryExpression
ChannelWrite <- BinaryExpression

IndexedGet <- BinaryExpression
```

```
class Dot(rval = true, lval = true) : Expression
    + primary: Expression *
    + field: NString *
```

```
class IfExpression(rval = true, lval = false) : Expression
    + initializer: Statement *
    + condition: Expression *
    + else_clause: Expression
    + then_clause: Statement *
```

```
class WhenExpression : Expression
    + initializer: Statement *
    + condition: Expression *
    class CaseClause : Node
        + case_: {Casting, Expression}
        + then_: Statement
    + case_clauses: List<CaseClause *>
    + else_clause: Statement
```

```
class Casting : Expression
    + primary: Expression *
    + destination: Type *
```

```
class Calling : Expression
    + callee: Expression *
    + args: List<Expression *>
```

### Type Reference

```
class Type : Node
    = Sign(buf: char *, n: size_t)
    = IsArray(): bool
    = IsClass(): bool
    = IsFunction(): bool
    = AsArray(): ArrayType *
    = AsClass(): ClassType *
    = AsFunction(): FunctionPrototype *
    = ToArray(dimension_count: int): ArrayType *
    = ToClass(class_def: ClassDefinition *): ClassType *
    = ToFunction(...): FunctionPrototype *
    + primary_type: {t_unit, t_bool, t_i8, t_u8, t_i16, ..., t_string, t_chan, t_fun, t_class, t_struct, t_identifier, t_array}
    + identifier: Symbol *
    + generic_args: List<Type *>
```

```
class ArrayType : Type
    + dimension_count: int
```

```
class ClassType: Type
    + class_def: ClassDefinition *
```

```
class InterfaceType: Type
    + interface_def: InterfaceDefinition *
```

```
class FunctionPrototype : Type
    + parameters: List<VariableDeclaration::Item *>
    + vargs: bool
    + returns: List<Type *>
```

## Lowing

### Base of Low Nodes

```
class Operand : Node
    + location: {kUnkown, kArguemnt, kRegister, kFPRegister, kStackSlot, kMemory, kConstant}
    + index: int64_t // == kUnallocated
```

### Memory

```
class Argument : Operand
    + type: Type *

class StackAllocate : Operand
    + type: Type *

class HeapAllocate: Operand
    + type: Type *

class StackStore : Operand
    + destination: Operand *

class GlobalLoad : Operand
    + symbol: NString *

class GlobalStore : Operand
    + symbol: NString *
    + destination: Operand *
```

### Constant

```
class Immediate<T> : Operand
    + value: T

class Constant<T> : Operand
    + value: T
```

### Call Runtime

```
class CallRuntime : Operand
    + fun: RuntimeFunCode
    + arguments: List<Operand *>
```

### Arithmetic

```
class Arithmetic<N> : Operand
    + operands: Operand*[N]
    + bits: int

I8Add <- Arithmetic<2>
U8Add <- Arithmetic<2>
...
I32Add <- Arithmetic<2>
U32Add <- Arithmetic<2>
...
I32Sub <- Arithmetic<2>
U32Sub <- Arithmetic<2>
...

```

Lowing example:

```
Example 1: 1 + foo.number

              +-------+                                         +--------+
              |  Add  |                                         | I32Add |
              +-------+                                         +--------+
             /         \                                        /         \
            /           \                              +----------+        \
+------------+          +----------+                   | Constant |      +--------------+
| IntLiteral |          |    Dot   |     lowing        |    1     |      | GlobalLoad   |
|  value(1)  |          | "number" |  =============>   +----------+      | "foo.number" |
+------------+          +----------+                                     +--------------+
                      /
              +------------+
              | Identifier |
              |   "foo"    |
              +------------+

Example 2:  
var i = 0
while (i < 10) { i = i + 1 }

                         +----------------+
                         | WhileStatement |
                         +----------------+
                        /                  \
               +------+                      +-----------+
               | Less |                      |   Block   |
               +------+                      +-----------+
              /        \                                  \
+------------+          +------------+                     +------------+
| Identifier |          | IntLiteral |                     | Assignment |
|    "i"     |          |     10     |                     +------------+
+------------+          +------------+                    /              \
                                                         /                \
                                           +------------+                  +-------+
                                           | Identifier |                  |  Add  |
                                           |     "i"    |                  +-------+
                                           +------------+                 /         \
                                                                         /           \
                                                           +------------+             +------------+
                                                           | Identifier |             | IntLiteral |
                                                           |    "i"     |             |      1     |
                                                           +------------+             +------------+
Lowing
=====>

                        +---------------+
                        | ConditionLoop |
                        +---------------+
                       /                 \
             +---------+                  +-----------+
             | I32Less |                  | BascBlock |
             +---------+                  +-----------+
            /           \                        \
+ - - - - - - - +    +----------+         +------------+
| StackAllocate |    | Constant |         | StackStore |
|      [0]      |    |    10    |         |    [0]     |
+ - - - - - - - +    +----------+         +------------+
                                                \
                                            +--------+
                                            | I32Add |
                                            +--------+
                                            /        \
                            + - - - - - - - +        +----------+
                            | StackAllocate |        | Constant |
                            |      [0]      |        |     1    |
                            + - - - - - - - +        +----------+
```