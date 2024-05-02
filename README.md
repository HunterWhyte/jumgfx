# **JumGFX**
Created for the video game Jum; the goal of this framework is to allow users/modders extreme flexibility in creating their own 2D graphics for a game. This means we have no “hardcoded” graphics, everything is defined in data files editable by users.

Demo of something created with this framework:
https://github.com/HunterWhyte/jumgfx/assets/61810551/2b3cc722-5841-43a1-bc02-fefc02699bf5

## Overview
In Below is a brief overview of the main functionality of JumGFX.

The basic architecture is that the game provides some input in the form of numerical values. The user defines a series of "items" which take the inputs from the game and transform them to output graphics on the screen in the form of textured quads, shaders, and text.

`[game model] -> [serialized input] -> [items and layers] -> [rendered output]`

### Inputs
The game defines a set of inputs that are updated according to the game state, for example if there is a player on screen the game may define a `PLAYER_POS` input. Which can then be used by the user to define where to draw an item on screen.

### Items

Generally, _items_ are things that the framework draws on the screen. _Items_ can be thought of as the output of the framework. The most basic type of item is a _textured item._ A _textured item_ has multiple _fields_ including: `position`, `size`, `path to texture`.

If we wanted to draw an image of size 100 where the player is we would do
```
position = PLAYER_POS
size = 100
```

### Expressions
Directly using _inputs_ as the _fields_ of _items_ is very limited. In order to be as flexible and powerful as possible the user is allowed to define _expressions_ to be used as the _fields_ of _items_. 

For example using the `position` and `size` _fields_ of a textured item we can write the following expressions:
```
position = (PLAYER_POS * 100) + 5
size = (BLOCK_SIZE* 4 )
```
Which will be evaluated as shown.

## Design Details
### Nodes
The basic building blocks of the framework are _nodes_. A _node_ performs an operation or holds a value. There are a number of arithmetic and comparison operators available.

Expressions are parsed into trees of nodes. If we were to write the expression `PLAYER_POS + 1`

This would get parsed into.
```
  [Addition Operator]
  /                 \
[PLAYER_POS]        [1]
```
Then evaluating the addition operator node would result in the value of `PLAYER_POS` + the literal value `1`.

### Node Lengths

In the above example we were talking about drawing a texture at the position of the player. When there is only one player this is a single value. However we must accommodate for the scenario where there are many multiples of one type of item. For example if we want to draw a block texture at every instance of a block in a tetris-like puzzle game.

The way that this is handled is that each leaf node can actually hold 0 or any number of values. The number of values in a node is the node’s _length_. The node’s length can change every frame, when the expression parsing occurs it is evaluating however many values there are and feeding that to the items.

There are a few rules defined on how this is handled, but the end result is that you can write something like:

```
position = BLOCK_POS
size = 50
```

And the framework will draw a texture for every block on the screen.

### Events and Animations

The serialized game input is stateless. The node evaluation happens every time there is a change in the game state. However it is useful to be able to trigger animations based on events that play over time. The game also inputs _events_ to the framework - events an example of an event is every time a user presses the jump button we emit a `JUMP` event, then play a jumping animation animation.

Animations generate nodes with values that range between 0-1 and are triggered off of events.

Animations are fully user defined.

The user is required to specify a name/key for the node, the length of the animation, and the event for which to trigger the animation on. Example

```
length = 0.3s
event = JUMP
```

### Layers

Items are grouped into layers. The main purpose of layers is that custom shaders are applied on the layer level. It is also useful for organizing and ordering the order in which items should be drawn. There are other useful features that can be defined per layer, such as:
- Masking out edges so that only items within a certain region will be drawn
- Applying an offset and scale to all items within a layer

### Related Nodes

Each leaf node can represent a channel for a single output from the game, a single float value. However, most outputs from the game have multiple related values. For example a block will have its x position and y position. If we want to draw a block. Therefore we have groups of nodes that we call _inputs_.

For example the input into the framework for a block would look like
```
BLOCK
  x position
  y position
```

Where x and y position are nodes.

**Mixing Node Lengths**

As covered in the Node Lengths section, there can be multiple values per node. For example the above _input_ might look like this

```
BLOCK
  x position = [5, 20, 40..]
  y position = [0, 10, 8..]
```

This represents three blocks at positions `(5,0), (20,10), (40,8)`. Since the x and y positions are stored in different nodes. If we want to draw an item at all three positions the output needs to index through the arrays consistently. There are set rules for how this happens.

The first thing to mention is that the operators mentioned earlier have a main argument and a secondary argument. The indexing is performed based on the main argument

So for example if we took

```
BLOCK
  x position = [5, 20, 40..]
  y position = [0, 10, 8…]
```

and fed it into an expression of `(BLOCK.x + BLOCK.y)`. The output would be `[5, 30, 48]`.

So far I have been expressing the operators as their algebraic equivalent, however they are not strictly commutative when the nodes have different lengths. The above expression `(BLOCK.x + BLOCK.y)` may be more clearly stated as `add(BLOCK.x, BLOCK.y)` where `BLOCK.x` is the main argument and `BLOCK.y` is the secondary.

When the nodes have different lengths the indexing is done based off the main argument. If the secondary argument has less values then its last value will be extended. For example if we have two properties like
```
a = [0, 1, 2, 3, 4]
b = [5, 3]
```
And do the expression add(a, b) this will be evaluated to
```
add(a, b) =  [0 + 5,
              1 + 3,
              2 + 3,
              3 + 3,
              4 + 3]
```

The reverse gives a different result, it will evaluate to the length of `b` and truncate the rest of `a`

```
add(b, a) = [5 + 0,
             3 + 0]
```

**Zero Length Nodes**

As previously mentioned, the expression node trees are built at parsing and remain static. However it is often the case that a node from an input will have no values. The rules for how this is handled is defined on a per operator basis. But the general rule is that if the secondary argument has no value then the operator should leave the main argument unchanged. For example addition of `add(a, b)`

```
a = [0, 1, 2, 3, 4]
b  = []
```

will be evaluated to
```
add(a, b) = [0 + 0,
             1 + 0,
             2 + 0,
             3 + 0,
             4 + 0]
```

But addition `add(b, a)`

evaluates to empty
```
add(b, a) = []
```

**Mixing Node Lengths of Item Fields**

Since items have multiple fields it raises the question of how do you decide how many items to draw. For example if you had an item with fields that evaluate to:
```
position.x = [0, 1, 2, 3, 4]
position.y = [0, 1, 2, 3, 4]
size = [5]
```

This is solved simply by assigning a field as the _indexer_, meaning all other fields will be indexed based on the length of this field. By default this is the x position. So the above item would draw 5 items at

`(0,0) (1,1) (2,2) (3,3) (4,4)` all using the first value of the size field.

If a given field is evaluated to be length 0, it is assigned a value of 0.

**Text Representation**

A _skin_ is a collection of assets (textures, shaders, etc.) and a single text file that defines all of the items, layers, animations etc. When opened the framework parses the text file and loads the assets. The text file is serialized in yaml format.

There are a number of quality of life features built in to the text representation. One of which is that nodes can be defined by the user and reused in multiple places. If you have a duplicate expression in multiple spots it can be pulled out to its own node and then referenced.

**Skin Editor**

There is also a WIP graphical editor that loads and modifies the text representation. It features menus for defining expressions, previewing items, and inspecting inputs and node trees. A full node based editor is in the works which is much clearer to work with when creating expressions.

**Input Implementation Details**

An input is just a named group of nodes. The user defines the input in code but we also want the definition to hold description of the input and its properties. Each input should be able to label its nodes whatever it wants. The user also can update the values in the input node however they want. The framework core does not care about how the handles for the inputs are stored and accessed since they only hold pointers to the nodes which are allocated within the skin. What is important is the naming of the nodes since that is how the lookup happens at the parsing step.

On skin_init we need to also pass the array of inputs that we want to use as inputs to the framework. At that point it will iterate through all the inputs and their nodes and allocate and assign nodes.

**<span style="text-decoration:underline;">SPECIFICATION - WIP</span>**

```
Items
  Textured
  Textured Animation
  Text
  Shape
  Video

Shaders
  Vertex
  Fragment
  Uniforms
```
