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
        +- ThrowStatement
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
                +- LambdaLiteral
                    +- IntLiteral
                    +- UIntLiteral
                    +- I64Literal
                    +- U64Literal
                    +- BoolLiteral
                    +- StringLiteral
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
            +- TryCatchExpression
            +- ChannelInitializer
            +- ArrayInitializer
            +- ArrayDimension
    +- Annotation
    +- Type
        +- ArrayType
        +- ClassType
        +- InterfaceType
        +- FunctionPrototype
    +- Operand
        +- Argument
        +- StackAllocate
        +- HeapAllocate
        +- StackLoad
        +- StackStore
        +- GlobalLoad
        +- GlobalStore
        +- CallRuntime
        +- CallDirect
        +- CallIndirect
        +- CallVirtual
        +- Arithmetic<N>
            +- I8{Add/Sub/Mul/Div/Mod/And/Or/Not/Xor/Inv/Shl/Sar}
            +- U8{Add/Sub/Mul/Div/Mod/And/Or/Not/Xor/Inv/Shl/Shr}
            +- I16{Add/Sub/Mul/Div/Mod/And/Or/Not/Xor/Inv/Shl/Sar}
            +- U16{Add/Sub/Mul/Div/Mod/And/Or/Not/Xor/Inv/Shl/Shr}
            +- I32{Add/Sub/Mul/Div/Mod/And/Or/Not/Xor/Inv/Shl/Sar}
            +- U32{Add/Sub/Mul/Div/Mod/And/Or/Not/Xor/Inv/Shl/Shr}
            +- I64{Add/Sub/Mul/Div/Mod/And/Or/Not/Xor/Inv/Shl/Sar}
            +- U64{Add/Sub/Mul/Div/Mod/And/Or/Not/Xor/Inv/Shl/Shr}
            +- F32{Add/Sub/Mul/Div}
            +- F64{Add/Sub/Mul/Div}
```


## Base Node

* Class Node is arean object.
* Only allocate from arean.
* All AST/IL nodes base class

```c++
class Node
    = Accept(NodeVisitor*): Visitation // Accept visitor
    = IsXXX(): bool // Type checker
    ...
    = AsXXX(): XXX* // Type casting
    ...
    + kind: Kind // Kind of node
    + source_position: SourcePosition // Source file position
```

```c++
class SourcePosition
    + begin_column: int
    + begin_line: int
    + end_column: int
    + end_line: int
```

## AST Nodes

```c++
class FileUnit : Node
    + file_name: NString *
    + file_full_path: NString *
    + package_name: NString *
    + imports: List<ImportEntry> *
    + statements: List<Statement *> *
    + definitions: List<Definition *> *
    + declarations: List<Declaration *> *
```

```c++
class ImportEntry : Node
    + original_package_name: NString *
    + package_path: NString *
    + alias: NString *
```

```c++
class Declaration : Statement
    = Identifier(): Identifer *
    = Type(): TypeRef *
    = Item(int i): Declaration *
    = ItemSize(): size_t
    + annotations: List<Annotation *>
    + access: {kExport, kPublic, kProtected, kPrivate, kDefault}
```

```c++
class VariableDeclaration : Declaration
    + constraint: {kVal, kVar}
    + is_volatile: bool
    + variables: List<Item *>
    class Item : Declaration
        + identifer: Identifer *
        + type: Type *
    + initilaizers: List<Expression *>
```

```c++
class FunctionDeclaration : Declaration
    + name: NString *
    + decoration: {kNative, kAbstract, kOverride}
    + is_reduce: bool // 'fun foo(): xxx {...}' or 'fun foo() -> xxx' syntax
    + define_for: Definition *
    + defined: LambdaLiteral *
```

```c++
class ObjectDeclaration : Declaration
    + name: NString *
    + methods: List<FunctionDeclaration *>
    + members: List<VariableDeclaration *>
```

```c++
class Definition : Statement
    + annotations: List<Annotation *>
    + generic_params: List<GenericParameter *>
    + has_instantiated: bool
```

```c++
class GenericParameter : Node
    = Identifier(): Symbol *
    + identifier: Type *
    + constraint: Type *
```

```c++
class InterfaceDefinition : Definition
    + name: NString *
    + methods: List<FunctionDeclaration *>
```

```c++
class ClassDefinition : Definition
    + name: NString *
    + constructor: List<VariableDeclaration *> // class Foo(val i: int, j: int) {}
    + base: Type *
    + constraint: {kClass, kStruct}
    + super_calling: List<Expression *>
    + methods: List<FunctionDeclaration *>
    + members: List<VariableDeclaration *>
```

```c++
class Symbol : Node
    + prefix_name: NString *
    + name: NString *
```

### Annotation

```c++
class AnnotationDefinition : Definition
    + name: NString *
    + members: List<VariableDeclaration *>
```

```c++
class Annotation : Node
    + name: Symbol *
    class Field : Node
        + name: NString *
        + value: Expression *
    + fields: List<Field *>
```

### Statements

```c++
class Statement : Node
```

```c++
class Block : Statement
    + statements: List<Statement *>
```

```c++
class BreakStatement : Statement
```

```c++
class ContinueStatement : Statement
```

```c++
class ReturnStatement : Statement
    + values: List<Expression *>
```

```c++
class ThrowStatement : Statement
    + value: Expression *
```

```c++
class Assignment : Statement
    + lvals: List<Expression *>
    + rvals: List<Expression *>
```

```c++
class WhileStatement : Statement
    + condition: Expression *
    + body: Block *

// while (foo()>0) {
//    println("ok")
// }
```

```c++
class ForEachStatement : Statement
    + value: VariableDeclaration *
    + container: Expression *
```

```c++
class ForStepStatement : Statement
    + value: VariableDeclaration *
    + begin: Expression *
    + end: Expression *
    + bound: {kUntil, kTo}
```

### Expression

```c++
class Expression : Statement
    = is_lval(): bool
    = is_rval(): bool
    = is_only_lval(): bool
    = is_only_rval(): bool
    + is_lval: bool
    + is_rval: bool
```

```c++
class Identifer(rval = true, lval = true) : Expression
    + name: NString *
```

```c++
class Literal(rval = true, lval = false) : Expression
    = Type(type: Type *): Type *
    + type: Type *
```

```c++
class UnitLiteral : Literal
```

```c++
class ActualLiteral<T> : Literal
    + value: T

IntLiteral  <- ActualLiteral<int>
UIntLiteral <- ActualLiteral<unsigned>
I64Literal  <- ActualLiteral<int64_t>
U64Literal  <- ActualLiteral<uint64_t>
BoolLiteral <- ActualLiteral<bool>
StringLiteral <- ActualLiteral<NString *>
```

```c++
class LambdaLiteral : Literal
    + prototype: FunctionPrototype *
    + body: { Block *| Expression *}
```

```c++
class ChannelInitializer(rval = true, lval = false) : Expression
    + generic_arg: Type *
    + capacity: Expression *
```

```c++
class ArrayInitializer(rval = true, lval = false) : Expression
    = dimension_count(): int
    + hint_type: ArrayType *
    + dimension: List<ArrayDimension *>

class ArrayDimension(rval = true, lval = false) : Expression
    + values: List<ArrayDimension *| Expression *>
```

```c++
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

```c++
class Dot(rval = true, lval = true) : Expression
    + primary: Expression *
    + field: NString *
```

```c++
class IfExpression(rval = true, lval = false) : Expression
    + initializer: Statement *
    + condition: Expression *
    + else_clause: Expression
    + then_clause: Statement *
```

```c++
class WhenExpression : Expression
    + initializer: Statement *
    + condition: Expression *
    class CaseClause : Node
        + case_: {Casting, Expression}
        + then_: Statement
    + case_clauses: List<CaseClause *>
    + else_clause: Statement
```

```c++
class TryCatchExpression : Expression
    + try_block: Block *
    class CatchClause : Node
        + pattern: Casting *
        + handler: Block *
    + catch_clauses: List<CatchClause *>
    + finally_clause: Block *
```

```c++
class Casting : Expression
    + primary: Expression *
    + destination: Type *
```

```c++
class Calling : Expression
    + callee: Expression *
    + args: List<Expression *>
```

### Type Reference

__Type Code__

* Unit: `T_unit`
* Bool: `T_bool`
* Integral: `T_i8` `T_u8` `T_i16` `T_u16` `T_int` `T_uint` `T_i64` `T_u64` `T_char`
* Floating: `T_f32` `T_f64`
* Others: `T_fun` `T_chan` `T_array` `T_string` `T_class` `T_struct` `T_identifier`


```c++
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
    + primary_type: {T_unit, T_bool, T_i8, T_u8, T_i16, ..., T_string, T_chan, T_fun, T_class, T_struct, T_identifier, T_array}
    + identifier: Symbol *
    + generic_args: List<Type *>
```

```c++
class ArrayType : Type
    + dimension_count: int
```

```c++
class ClassType: Type
    + class_def: ClassDefinition *
```

```c++
class InterfaceType: Type
    + interface_def: InterfaceDefinition *
```

```c++
class FunctionPrototype : Type
    + parameters: List<VariableDeclaration::Item *>
    + vargs: bool
    + returns: List<Type *>
```

## Lowing

### Base of Low Nodes

```c++
class Operand : Node
    + location: {kUnit, kArguemnt, kRegister, kFPRegister, kStackSlot, kMemory, kConstant}
    + index: int64_t // == kUnallocated
```

### Memory

```c++
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

```c++
class Immediate<T> : Operand
    + value: T

class Constant<T> : Operand
    + value: T
```

### Condition Branch

```c++

```

### Calls

```c++
class CallRuntime : Operand
    + fun: RuntimeFunCode
    + arguments: List<Operand *>

class CallDirect : Operand
    + symbol: NString *
    + arguments: List<Operand *>

class CallIndirect : Operand
    + fun: Operand *
    + arguments: List<Operand *>

class CallVirtual : Operand
    + self: Operand *
    + symbol: NString *
    + arguments: List<Operand *>
```

__Lowing__

```
Source: foo.bar(1, 2)

                +-----------------+                                      +------------+
                |     Calling     |                                      | CallDirect |
                +-----------------+                                      |  "foo.bar" |
               /        \          \                                     +------------+
      +-------+   +------------+   +------------+                        /             \
      |  Dot  |   | IntLiteral |   | IntLiteral |  lowing    +----------+               +----------+
      | "bar" |   |      1     |   |      2     | ========>  | Constant |               | Constant |
      +-------+   +------------+   +------------+            |     1    |               |     2    |
       /                                                     +----------+               +----------+
+------------+
| Identifier |
|    "foo"   |
+------------+
```

### Arithmetic

```c++
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