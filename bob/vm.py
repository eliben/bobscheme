#-------------------------------------------------------------------------------
# bob: vm.py
#
# Implementation of the Bob virtual machine.
#
# Eli Bendersky (eliben@gmail.com)
# This code is in the public domain
#-------------------------------------------------------------------------------
from __future__ import print_function
from .bytecode import (
        OP_CONST, OP_LOADVAR, OP_STOREVAR, OP_DEFVAR, OP_FUNCTION, OP_POP,
        OP_JUMP, OP_FJUMP, OP_RETURN, OP_CALL)
from .expr import expr_repr, Boolean
from .builtins import BuiltinProcedure, builtins_map
from .environment import Environment
from .utils import Stack


DEBUG = False


class Closure(object):
    """ A closure is a code object (procedure) with an associated environment
        in which the closure was defined.
    """
    def __init__(self, codeobject, env):
        self.codeobject = codeobject
        self.env = env


class ExecutionFrame(object):
    """ Encapsulates the execution "frame": state of the VM. 
    
        codeobject: 
            The current code object executed by the VM
        pc: 
            An index into the code object of the next instruction to execute
        env: 
            The environment in which the code is being executed
    """
    def __init__(self, codeobject, pc, env):
        self.codeobject = codeobject
        self.pc = pc
        self.env = env


class BobVM(object):
    """ Implementation of the Bob VM. Initialize and then .run() with a 
        CodeObject.
    """
    class VMError(Exception): pass
        
    def __init__(self, output_stream=None):
        self.valuestack = Stack()
        self.framestack = Stack()
        
        self.frame = ExecutionFrame(
                        codeobject=None,
                        pc=None,
                        env=self._create_global_env())
        
        if output_stream is None:
            import sys
            self.output_stream = sys.stdout
        else:
            self.output_stream = output_stream
    
    def run(self, codeobject):
        """ The main execution function of the VM. Accepts a CodeObject, and
            runs it until there are no more instructions to execute.
        """
        self.frame.codeobject = codeobject
        self.frame.pc = 0
        
        #
        # The big VM loop!
        #
        while True:
            # If there are no more instructions in this code object, it's either 
            # because we're done executing the program, or an error.
            #
            instr = self._get_next_instruction()
            
            if instr is None:
                if self._is_in_toplevel_code():
                    break
                else:
                    raise self.VMError('Code object ended prematurely: %s' % self.codeobject)
            
            if DEBUG: print(opcode2str(instr.opcode), instr.arg)
            
            if instr.opcode == OP_CONST:
                self.valuestack.push(self.frame.codeobject.constants[instr.arg])
            elif instr.opcode == OP_LOADVAR:
                value = self.frame.env.lookup_var(self.frame.codeobject.varnames[instr.arg])
                self.valuestack.push(value)
            elif instr.opcode == OP_STOREVAR:
                value = self.valuestack.pop()
                self.frame.env.set_var_value(self.frame.codeobject.varnames[instr.arg], value)
            elif instr.opcode == OP_DEFVAR:
                value = self.valuestack.pop()
                self.frame.env.define_var(self.frame.codeobject.varnames[instr.arg], value)
            elif instr.opcode == OP_POP:
                if len(self.valuestack) > 0:
                    self.valuestack.pop()
            elif instr.opcode == OP_JUMP:
                self.frame.pc = instr.arg
            elif instr.opcode == OP_FJUMP:
                predicate = self.valuestack.pop()
                if isinstance(predicate, Boolean) and predicate.value is False:
                    self.frame.pc = instr.arg
            elif instr.opcode == OP_FUNCTION:
                func_codeobject = self.frame.codeobject.constants[instr.arg]
                closure = Closure(func_codeobject, self.frame.env)
                self.valuestack.push(closure)
            elif instr.opcode == OP_CALL:
                # For CALL what we have on TOS the function and then the 
                # arguments to pass to it - the last argument is highest on
                # the stack.
                # The function is either a BuiltinProcedure or a Closure (for
                # user-defined procedures)
                #
                proc = self.valuestack.pop()
                argvalues = [self.valuestack.pop() for i in range(instr.arg)]
                argvalues.reverse()
                
                if isinstance(proc, BuiltinProcedure):
                    result = proc.apply(argvalues)
                    self.valuestack.push(result)
                elif isinstance(proc, Closure):
                    if len(proc.codeobject.args) != len(argvalues):
                        raise self.VMError('Calling procedure %s with %s args, expected %s' % (
                                                proc.codeobject.name, len(args), len(proc.codeobject.args)))
                    
                    # We're now going to execute a code object, so save the
                    # current execution frame on the frame stack.
                    #
                    self.framestack.push(self.frame)

                    # Extend the closure's environment with the bindings of
                    # argument names --> passed values. 
                    #
                    arg_bindings = {}
                    for i, argname in enumerate(proc.codeobject.args):
                        arg_bindings[argname] = argvalues[i]
                    extended_env = Environment(arg_bindings, proc.env)
                    
                    # Start executing the procedure
                    #
                    self.frame = ExecutionFrame(
                                    codeobject=proc.codeobject,
                                    pc=0,
                                    env=extended_env)
                else:
                    raise self.VMError('Invalid object on TOS for CALL: %s' % proc)
                
            elif instr.opcode == OP_RETURN:
                self.frame = self.framestack.pop()
            else:
                raise self.VMError('Unknown instruction opcode: %s' % instr.opcode)
    
    def _get_next_instruction(self):
        """ Get the next instruction from the current code object and advance 
            PC. If the code object has no more instructions, return None.
        """
        if self.frame.pc >= len(self.frame.codeobject.code):
            return None
        else:
            instr = self.frame.codeobject.code[self.frame.pc]
            self.frame.pc += 1
            return instr
    
    def _is_in_toplevel_code(self):
        """ Is the VM currently executing the top-level code object?
        """
        return len(self.framestack) == 0
    
    def _create_global_env(self):
        global_binding = {}
        for name, func in builtins_map.items():
            global_binding[name] = BuiltinProcedure(name, func)

        # Add builtins which require access to the VM state.
        #
        global_binding['write'] = BuiltinProcedure('write', self._write)
        global_binding['debug-vm'] = BuiltinProcedure('debug-vm', self._hook_debug_vm)
        return Environment(global_binding)

    def _write(self, args):
        # Abides by the builtin procedure calling protocol - args is a Python
        # list of arguments.
        #
        self.output_stream.write(expr_repr(args[0]) + '\n')

    def _hook_debug_vm(self, args):
        print(self._show_vm_state())

    def _show_vm_state(self):
        def value_printer(item):
            if isinstance(item, Closure):
                return '| Closure <%s>\n' % item.codeobject.name
            elif isinstance(item, BuiltinProcedure):
                return '| BuiltinProcedure <%s>\n' % item.name
            else:
                return '| %s\n' % expr_repr(item)
        
        def frame_printer(item):
            str = 'Code: <%s>' % item.codeobject.name
            str += ' [PC=%s]\n' % item.pc
            return str
        
        def show_stack(stack, name, item_printer):
            str = ''
            head = '-' * (8 + len(name))
            str += '+%s+\n' % head
            str += '| %s stack |\n' % name
            str += '+%s+\n\n' % head
            
            i = 0
            while i < len(stack):
                str += '      |--------\n'
                if i == 0:
                    str += 'TOS:  '
                else:
                    str += '      '
                item = stack.peek(-1 - i)
                str += item_printer(item)
                i += 1
            str += '      |--------\n'
            return str

        str = show_stack(self.valuestack, 'Value', value_printer)
        str += '\n' + show_stack(self.framestack, 'Frame', frame_printer)
        return str


#------------------------------------------------------------------------------
if __name__ == '__main__':
    pass

