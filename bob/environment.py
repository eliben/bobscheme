#-------------------------------------------------------------------------------
# bob: environment.py
#
# Environment object. 
#
# Eli Bendersky (eliben@gmail.com)
# This code is in the public domain
#-------------------------------------------------------------------------------


class Environment(object):
    """ An environment in which variables are bound to values. Variable names
        must be hashable, values are arbitrary objects.
        
        Environment objects are linked via parent references. When bindings are
        queried or assigned and the variable name isn't bound in the 
        environment, the parent environment is recursively searched. 
        
        All environment chains ultimately terminate in a "top-level" environment
        which has None in its parent link.
    """
    class Unbound(Exception): pass
    
    def __init__(self, binding, parent=None):
        """ Create a new environment with the given binding (dict var -> value)
            and a reference to a parent environment.
        """
        self.binding = binding
        self.parent = parent
    
    def lookup_var(self, var):
        """ Looks up the bound value for the given variable, climbing up the
            parent reference if required. 
        """
        if var in self.binding:
            return self.binding[var]
        elif self.parent is not None:
            return self.parent.lookup_var(var)
        else:
            raise Environment.Unbound('unbound variable "%s"' % var)            

    def define_var(self, var, value):
        """ Add a binding of var -> value to this environment. If a binding for 
            the given var exists, it is replaced.
        """
        self.binding[var] = value
    
    def set_var_value(self, var, value):
        """ Sets the value of var. If var is unbound in this environment, climbs
            up the parent reference.
        """
        if var in self.binding:
            self.binding[var] = value
        elif self.parent is not None:
            self.parent.set_var_value(var, value)
        else:
            raise Environment.Unbound('unbound variable "%s"' % var)

