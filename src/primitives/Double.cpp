/*
 *
 *
 Copyright (c) 2007 Michael Haupt, Tobias Pape, Arne Bergmann
 Software Architecture Group, Hasso Plattner Institute, Potsdam, Germany
 http://www.hpi.uni-potsdam.de/swa/

 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:

 The above copyright notice and this permission notice shall be included in
 all copies or substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 THE SOFTWARE.
 */

#include <stdio.h>
#include <iostream>
#include <math.h>
#include <sstream>

#include <vmobjects/VMObject.h>
#include <vmobjects/VMFrame.h>
#include <vmobjects/VMDouble.h>
#include <vmobjects/VMString.h>
#include <vmobjects/VMInteger.h>
#include <vmobjects/VMBigInteger.h>

#include <vm/Universe.h>

#include "Double.h"
#include "../primitivesCore/Routine.h"

/*
 * This function coerces any right-hand parameter to a double, regardless of its
 * true nature. This is to make sure that all Double operations return Doubles.
 */
double _Double::coerceDouble(oop_t x) {
    if (IS_TAGGED(x))
        return (double) INT_VAL(x);
    
    pVMClass cl = ((AbstractVMObject*)x)->GetClass();
    if (cl == doubleClass)
        return static_cast<VMDouble*>(x)->GetEmbeddedDouble();
    else if(cl == integerClass)
        return (double)static_cast<pVMInteger>(x)->GetEmbeddedInteger();
    else if(cl == bigIntegerClass)
        return (double)static_cast<VMBigInteger*>(x)->GetEmbeddedInteger();
    else
        GetUniverse()->ErrorExit("Attempt to apply Double operation to non-number.");

    return 0.0f;
}

/*
 * The following standard functionality is performed in all arithmetic
 * operations: extract the right-hand operand by coercing it to a double, and
 * extract the left-hand operand as an immediate Double. Afterwards, left and
 * right are prepared for the operation.
 */
#define PREPARE_OPERANDS \
    double right = coerceDouble(frame->Pop()); \
    VMDouble* leftObj = static_cast<VMDouble*>(frame->Pop()); \
    double left = leftObj->GetEmbeddedDouble();

void _Double::Plus(pVMObject /*object*/, pVMFrame frame) {
    PREPARE_OPERANDS;
    frame->Push(GetUniverse()->NewDouble(left + right));
}

void _Double::Minus(pVMObject /*object*/, pVMFrame frame) {
    PREPARE_OPERANDS;
    frame->Push(GetUniverse()->NewDouble(left - right));
}

void _Double::Star(pVMObject /*object*/, pVMFrame frame) {
    PREPARE_OPERANDS;
    frame->Push(GetUniverse()->NewDouble(left * right));
}

void _Double::Slashslash(pVMObject /*object*/, pVMFrame frame) {
    PREPARE_OPERANDS;
    frame->Push(GetUniverse()->NewDouble(left / right));
}

void _Double::Percent(pVMObject /*object*/, pVMFrame frame) {
    PREPARE_OPERANDS;
    frame->Push(GetUniverse()->NewDouble((double)((int64_t)left %
                    (int64_t)right)));
}

void _Double::And(pVMObject /*object*/, pVMFrame frame) {
    PREPARE_OPERANDS;
    frame->Push(GetUniverse()->NewDouble((double)((int64_t)left &
                    (int64_t)right)));
}

void _Double::BitwiseXor(pVMObject /*object*/, pVMFrame frame) {
    PREPARE_OPERANDS;
    frame->Push(GetUniverse()->NewDouble((double)((int64_t)left ^
                    (int64_t)right)));
}

/*
 * This function implements strict (bit-wise) equality and is therefore
 * inaccurate.
 */
void _Double::Equal(pVMObject /*object*/, pVMFrame frame) {
    PREPARE_OPERANDS;
    if(left == right)
    frame->Push(trueObject);
    else
    frame->Push(falseObject);
}

void _Double::Lowerthan(pVMObject /*object*/, pVMFrame frame) {
    PREPARE_OPERANDS;
    if(left < right)
        frame->Push(trueObject);
    else
        frame->Push(falseObject);
}

void _Double::AsString(pVMObject /*object*/, pVMFrame frame) {
    VMDouble* self = static_cast<VMDouble*>(frame->Pop());

    double dbl = self->GetEmbeddedDouble();
    ostringstream Str;
    Str.precision(17);
    Str << dbl;
    frame->Push( GetUniverse()->NewString( Str.str().c_str() ) );
}

void _Double::Sqrt(pVMObject /*object*/, pVMFrame frame) {
    VMDouble* self = static_cast<VMDouble*>(frame->Pop());
    VMDouble* result = GetUniverse()->NewDouble( sqrt(self->GetEmbeddedDouble()) );
    frame->Push(result);
}

void _Double::Round(pVMObject /*object*/, pVMFrame frame) {
    VMDouble* self = (VMDouble*)frame->Pop();
    long int rounded = lround(self->GetEmbeddedDouble());

    // Check with integer bounds and push:
    if (rounded > INT32_MAX || rounded < INT32_MIN)
    frame->Push(GetUniverse()->NewBigInteger(rounded));
    else {
        frame->Push(NEW_INT((int32_t)rounded));
    }
}

_Double::_Double() : PrimitiveContainer() {
    SetPrimitive("plus",       new Routine<_Double>(this, &_Double::Plus));
    SetPrimitive("minus",      new Routine<_Double>(this, &_Double::Minus));
    SetPrimitive("star",       new Routine<_Double>(this, &_Double::Star));
    SetPrimitive("slashslash", new Routine<_Double>(this, &_Double::Slashslash));
    SetPrimitive("percent",    new Routine<_Double>(this, &_Double::Percent));
    SetPrimitive("and",        new Routine<_Double>(this, &_Double::And));
    SetPrimitive("equal",      new Routine<_Double>(this, &_Double::Equal));
    SetPrimitive("lowerthan",  new Routine<_Double>(this, &_Double::Lowerthan));
    SetPrimitive("asString",   new Routine<_Double>(this, &_Double::AsString));
    SetPrimitive("sqrt",       new Routine<_Double>(this, &_Double::Sqrt));
    SetPrimitive("bitXor_",    new Routine<_Double>(this, &_Double::BitwiseXor));
    SetPrimitive("round",      new Routine<_Double>(this, &_Double::Round));
}
