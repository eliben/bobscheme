#-------------------------------------------------------------------------------
# bob: parser.py
#
# Parser for a subset of Scheme. Follows the grammar defined in R5RS 
# 7.1.2 (External Representations).
#
# Eli Bendersky (eliben@gmail.com)
# This code is in the public domain
#-------------------------------------------------------------------------------
from .lexer import Lexer, Token, LexerError
from .expr import Pair, Number, Symbol, Boolean

class ParseError(Exception): pass


class BobParser(object):
    """ Recursive-descent parser.
    
        Since Scheme code is also data, this parser mimics the (read) procedure
        and reads source code into Scheme expressions (internal data 
        representation suitable for further analysis). 
    """
    def __init__(self):
        self.lexer = BobLexer()
        self.clear()
    
    def parse(self, text):
        """ Given a string with Scheme source code, parses it into a list of 
            expression objects.            
        """
        self.text = text
        self.lexer.input(self.text)
        self._next_token()
        return self._parse_file()
    
    def clear(self):
        """ Reset the parser's internal state.
        """
        self.text = ''
        self.cur_token = None

    def pos2coord(self, pos):
        """ Convert a lexing position (offset from start of text) into a 
            coordinate [line %s, column %s].
        """
        # Count the amount of newlines between the beginning of the parsed
        # text and pos. Then, count the column as an offset from the last 
        # newline
        #
        num_newlines = self.text.count('\n', 0, pos)
        line_offset = self.text.rfind('\n', 0, pos)
        if line_offset < 0:
            line_offset = 0
        return '[line %s, column %s]' % (num_newlines + 1, pos - line_offset)
    
    ######################--   PRIVATE   --######################
    
    def _parse_error(self, msg, token=None):
        token = token or self.cur_token
        if token:
            coord = self.pos2coord(token.pos)
            raise ParseError('%s %s' % (msg, coord))
        else:
            raise ParseError(msg)
    
    def _next_token(self):
        try:
            while True:
                self.cur_token = self.lexer.token()
                if self.cur_token is None or self.cur_token.type != 'COMMENT':
                    break
        except LexerError as lexerr:
            raise ParseError('syntax error at %s' % self.pos2coord(lexerr.pos))
        
    def _match(self, type):
        """ The 'match' primitive of RD parsers. 
        
            * Verifies that the current token is of the given type 
            * Returns the value of the current token
            * Reads in the next token
        """
        if self.cur_token.type == type:
            val = self.cur_token.val
            self._next_token()
            return val
        else:
            self._parse_error('Unmatched %s (found %s)' % (type, self.cur_token.type))
    
    ##
    ## Recursive parsing rules. The top-level is _parse_file, which expects
    ## a sequence of Scheme expressions. The rest of the rules follow section
    ## 7.1.2 of R5RS with some re-ordering for programming convenience.
    ##
    def _parse_file(self):
        datum_list = []
        while self.cur_token:
            datum_list.append(self._datum())
        return datum_list
        
    def _datum(self):
        # list
        if self.cur_token.type == 'LPAREN':
            return self._list()
        # abbreviation
        elif self.cur_token.type == 'QUOTE':
            return self._abbreviation()
        # simple datum
        else:
            return self._simple_datum()
    
    def _simple_datum(self):
        if self.cur_token.type == 'BOOLEAN':
            retval = Boolean(self.cur_token.val == '#t')
        elif self.cur_token.type == 'NUMBER':
            base = 10
            num_str = self.cur_token.val
            if num_str.startswith('#'):
                if num_str[1] == 'x': base = 16
                elif num_str[1] == 'o': base = 8
                elif num_str[1] == 'b': base = 2
                num_str = num_str[2:]
            
            try:
                retval = Number(int(num_str, base))
            except ValueError as err:
                self._parse_error('Invalid number')
        elif self.cur_token.type == 'ID':
            retval = Symbol(self.cur_token.val)
        else:
            self._parse_error('Unexpected token "%s"' % self.cur_token.val)
        
        self._next_token()
        return retval
    
    def _list(self):
        # Algorithm:
        #
        # 1. First parse all sub-datums into a sequential Python list.
        # 2. Convert this list into nested Pair objects
        #
        # To handle the dot ('.'), dot_idx keeps track of the index in lst
        # where the dot was specified.
        # 
        self._match('LPAREN')
        lst = []
        dot_idx = -1
        
        while True:
            if not self.cur_token:
                self._parse_error('Unmatched parentheses at end of input')
            elif self.cur_token.type == 'RPAREN':
                break
            elif self.cur_token.type == 'ID' and self.cur_token.val == '.':
                if dot_idx > 0:
                    self._parse_error('Invalid usage of "."')
                dot_idx = len(lst)
                self._match('ID')
            else:
                lst.append(self._datum())
        
        # Figure out whether we have a dotted list and whether the dot was 
        # placed correctly
        #
        dotted_end = False
        if dot_idx > 0:
            if dot_idx == len(lst) - 1:
                dotted_end = True
            else:
                self._parse_error('Invalid location for "." in list')
        
        self._match('RPAREN')
        
        if dotted_end:
            cur_cdr = lst[-1]
            lst = lst[:-1]
        else:
            cur_cdr = None
            
        for datum in reversed(lst):
            cur_cdr = Pair(datum, cur_cdr)
        
        return cur_cdr
    
    def _abbreviation(self):
        quotepos = self.cur_token.pos
        self._match('QUOTE')
        datum = self._datum()
        return Pair(Symbol('quote'), Pair(datum, None))


class BobLexer(Lexer):
    """ Partial Scheme lexer based on R5RS 7.1.1 (Lexical structure).
    """
    def __init__(self):
        rules = self._lexing_rules()
        super(BobLexer, self).__init__(rules, skip_whitespace=True)
    
    def _lexing_rules(self):
        # Regex helpers
        #
        digit_2 = r'[0-1]'
        digit_8 = r'[0-7]'
        digit_10 = r'[0-9]'
        digit_16 = r'[0-9A-Fa-f]'
        
        radix_2 = r'\#b'
        radix_8 = r'\#o'
        radix_10 = r'(\#d)?'
        radix_16 = r'\#x'
        
        number = r'(%s%s+|%s%s+|%s%s+|%s%s+)' %(
                        radix_2, digit_2,
                        radix_8, digit_8,
                        radix_10, digit_10,
                        radix_16, digit_16,)
        
        special_initial = r'[!$%&*.:<=>?^_~]'
        initial = '([a-zA-Z]|'+special_initial+')'
        special_subsequent = r'[+-.@]'
        subsequent = '(%s|%s|%s)' % (initial, digit_10, special_subsequent)
        
        peculiar_identifier = r'([+\-.]|\.\.\.)'
        identifier = '(%s%s*|%s)' % (initial, subsequent, peculiar_identifier)

        rules = [
            (r';[^\n]*',                'COMMENT'),
            (r'\#[tf]',                 'BOOLEAN'),
            (number,                    'NUMBER'),
            (identifier,                'ID'),
            (r'\(',                     'LPAREN'),
            (r'\)',                     'RPAREN'),
            (r'\'',                     'QUOTE'),
        ]
        return rules


#-------------------------------------------------------------------------------
if __name__ == '__main__':
    pass

