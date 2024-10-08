// Generated from /u/wjl4jj/sipc-lewis_zhang/src/frontend/TIP.g4 by ANTLR 4.13.1
import org.antlr.v4.runtime.Lexer;
import org.antlr.v4.runtime.CharStream;
import org.antlr.v4.runtime.Token;
import org.antlr.v4.runtime.TokenStream;
import org.antlr.v4.runtime.*;
import org.antlr.v4.runtime.atn.*;
import org.antlr.v4.runtime.dfa.DFA;
import org.antlr.v4.runtime.misc.*;

@SuppressWarnings({"all", "warnings", "unchecked", "unused", "cast", "CheckReturnValue", "this-escape"})
public class TIPLexer extends Lexer {
	static { RuntimeMetaData.checkVersion("4.13.1", RuntimeMetaData.VERSION); }

	protected static final DFA[] _decisionToDFA;
	protected static final PredictionContextCache _sharedContextCache =
		new PredictionContextCache();
	public static final int
		T__0=1, T__1=2, T__2=3, T__3=4, T__4=5, T__5=6, T__6=7, T__7=8, T__8=9, 
		T__9=10, T__10=11, T__11=12, T__12=13, T__13=14, T__14=15, MUL=16, DIV=17, 
		MOD=18, PLUSPLUS=19, ADD=20, MINUSMINUS=21, SUB=22, GE=23, GT=24, LE=25, 
		LT=26, EQ=27, NE=28, NUMBER=29, KALLOC=30, KINPUT=31, KWHILE=32, KIF=33, 
		KFOR=34, KELSE=35, KVAR=36, KRETURN=37, KNULL=38, KOUTPUT=39, KERROR=40, 
		KOF=41, KBY=42, TRUE=43, FALSE=44, AND=45, OR=46, NOT=47, KPOLY=48, IDENTIFIER=49, 
		WS=50, BLOCKCOMMENT=51, COMMENT=52;
	public static String[] channelNames = {
		"DEFAULT_TOKEN_CHANNEL", "HIDDEN"
	};

	public static String[] modeNames = {
		"DEFAULT_MODE"
	};

	private static String[] makeRuleNames() {
		return new String[] {
			"T__0", "T__1", "T__2", "T__3", "T__4", "T__5", "T__6", "T__7", "T__8", 
			"T__9", "T__10", "T__11", "T__12", "T__13", "T__14", "MUL", "DIV", "MOD", 
			"PLUSPLUS", "ADD", "MINUSMINUS", "SUB", "GE", "GT", "LE", "LT", "EQ", 
			"NE", "NUMBER", "KALLOC", "KINPUT", "KWHILE", "KIF", "KFOR", "KELSE", 
			"KVAR", "KRETURN", "KNULL", "KOUTPUT", "KERROR", "KOF", "KBY", "TRUE", 
			"FALSE", "AND", "OR", "NOT", "KPOLY", "IDENTIFIER", "WS", "BLOCKCOMMENT", 
			"COMMENT"
		};
	}
	public static final String[] ruleNames = makeRuleNames();

	private static String[] makeLiteralNames() {
		return new String[] {
			null, "'('", "','", "')'", "'{'", "'}'", "';'", "'['", "']'", "'.'", 
			"'#'", "'&'", "'?'", "':'", "'='", "'..'", "'*'", "'/'", "'%'", "'++'", 
			"'+'", "'--'", "'-'", "'>='", "'>'", "'<='", "'<'", "'=='", "'!='", null, 
			"'alloc'", "'input'", "'while'", "'if'", "'for'", "'else'", "'var'", 
			"'return'", "'null'", "'output'", "'error'", "'of'", "'by'", "'true'", 
			"'false'", "'and'", "'or'", "'not'", "'poly'"
		};
	}
	private static final String[] _LITERAL_NAMES = makeLiteralNames();
	private static String[] makeSymbolicNames() {
		return new String[] {
			null, null, null, null, null, null, null, null, null, null, null, null, 
			null, null, null, null, "MUL", "DIV", "MOD", "PLUSPLUS", "ADD", "MINUSMINUS", 
			"SUB", "GE", "GT", "LE", "LT", "EQ", "NE", "NUMBER", "KALLOC", "KINPUT", 
			"KWHILE", "KIF", "KFOR", "KELSE", "KVAR", "KRETURN", "KNULL", "KOUTPUT", 
			"KERROR", "KOF", "KBY", "TRUE", "FALSE", "AND", "OR", "NOT", "KPOLY", 
			"IDENTIFIER", "WS", "BLOCKCOMMENT", "COMMENT"
		};
	}
	private static final String[] _SYMBOLIC_NAMES = makeSymbolicNames();
	public static final Vocabulary VOCABULARY = new VocabularyImpl(_LITERAL_NAMES, _SYMBOLIC_NAMES);

	/**
	 * @deprecated Use {@link #VOCABULARY} instead.
	 */
	@Deprecated
	public static final String[] tokenNames;
	static {
		tokenNames = new String[_SYMBOLIC_NAMES.length];
		for (int i = 0; i < tokenNames.length; i++) {
			tokenNames[i] = VOCABULARY.getLiteralName(i);
			if (tokenNames[i] == null) {
				tokenNames[i] = VOCABULARY.getSymbolicName(i);
			}

			if (tokenNames[i] == null) {
				tokenNames[i] = "<INVALID>";
			}
		}
	}

	@Override
	@Deprecated
	public String[] getTokenNames() {
		return tokenNames;
	}

	@Override

	public Vocabulary getVocabulary() {
		return VOCABULARY;
	}


	public TIPLexer(CharStream input) {
		super(input);
		_interp = new LexerATNSimulator(this,_ATN,_decisionToDFA,_sharedContextCache);
	}

	@Override
	public String getGrammarFileName() { return "TIP.g4"; }

	@Override
	public String[] getRuleNames() { return ruleNames; }

	@Override
	public String getSerializedATN() { return _serializedATN; }

	@Override
	public String[] getChannelNames() { return channelNames; }

	@Override
	public String[] getModeNames() { return modeNames; }

	@Override
	public ATN getATN() { return _ATN; }

	public static final String _serializedATN =
		"\u0004\u00004\u0130\u0006\uffff\uffff\u0002\u0000\u0007\u0000\u0002\u0001"+
		"\u0007\u0001\u0002\u0002\u0007\u0002\u0002\u0003\u0007\u0003\u0002\u0004"+
		"\u0007\u0004\u0002\u0005\u0007\u0005\u0002\u0006\u0007\u0006\u0002\u0007"+
		"\u0007\u0007\u0002\b\u0007\b\u0002\t\u0007\t\u0002\n\u0007\n\u0002\u000b"+
		"\u0007\u000b\u0002\f\u0007\f\u0002\r\u0007\r\u0002\u000e\u0007\u000e\u0002"+
		"\u000f\u0007\u000f\u0002\u0010\u0007\u0010\u0002\u0011\u0007\u0011\u0002"+
		"\u0012\u0007\u0012\u0002\u0013\u0007\u0013\u0002\u0014\u0007\u0014\u0002"+
		"\u0015\u0007\u0015\u0002\u0016\u0007\u0016\u0002\u0017\u0007\u0017\u0002"+
		"\u0018\u0007\u0018\u0002\u0019\u0007\u0019\u0002\u001a\u0007\u001a\u0002"+
		"\u001b\u0007\u001b\u0002\u001c\u0007\u001c\u0002\u001d\u0007\u001d\u0002"+
		"\u001e\u0007\u001e\u0002\u001f\u0007\u001f\u0002 \u0007 \u0002!\u0007"+
		"!\u0002\"\u0007\"\u0002#\u0007#\u0002$\u0007$\u0002%\u0007%\u0002&\u0007"+
		"&\u0002\'\u0007\'\u0002(\u0007(\u0002)\u0007)\u0002*\u0007*\u0002+\u0007"+
		"+\u0002,\u0007,\u0002-\u0007-\u0002.\u0007.\u0002/\u0007/\u00020\u0007"+
		"0\u00021\u00071\u00022\u00072\u00023\u00073\u0001\u0000\u0001\u0000\u0001"+
		"\u0001\u0001\u0001\u0001\u0002\u0001\u0002\u0001\u0003\u0001\u0003\u0001"+
		"\u0004\u0001\u0004\u0001\u0005\u0001\u0005\u0001\u0006\u0001\u0006\u0001"+
		"\u0007\u0001\u0007\u0001\b\u0001\b\u0001\t\u0001\t\u0001\n\u0001\n\u0001"+
		"\u000b\u0001\u000b\u0001\f\u0001\f\u0001\r\u0001\r\u0001\u000e\u0001\u000e"+
		"\u0001\u000e\u0001\u000f\u0001\u000f\u0001\u0010\u0001\u0010\u0001\u0011"+
		"\u0001\u0011\u0001\u0012\u0001\u0012\u0001\u0012\u0001\u0013\u0001\u0013"+
		"\u0001\u0014\u0001\u0014\u0001\u0014\u0001\u0015\u0001\u0015\u0001\u0016"+
		"\u0001\u0016\u0001\u0016\u0001\u0017\u0001\u0017\u0001\u0018\u0001\u0018"+
		"\u0001\u0018\u0001\u0019\u0001\u0019\u0001\u001a\u0001\u001a\u0001\u001a"+
		"\u0001\u001b\u0001\u001b\u0001\u001b\u0001\u001c\u0004\u001c\u00aa\b\u001c"+
		"\u000b\u001c\f\u001c\u00ab\u0001\u001d\u0001\u001d\u0001\u001d\u0001\u001d"+
		"\u0001\u001d\u0001\u001d\u0001\u001e\u0001\u001e\u0001\u001e\u0001\u001e"+
		"\u0001\u001e\u0001\u001e\u0001\u001f\u0001\u001f\u0001\u001f\u0001\u001f"+
		"\u0001\u001f\u0001\u001f\u0001 \u0001 \u0001 \u0001!\u0001!\u0001!\u0001"+
		"!\u0001\"\u0001\"\u0001\"\u0001\"\u0001\"\u0001#\u0001#\u0001#\u0001#"+
		"\u0001$\u0001$\u0001$\u0001$\u0001$\u0001$\u0001$\u0001%\u0001%\u0001"+
		"%\u0001%\u0001%\u0001&\u0001&\u0001&\u0001&\u0001&\u0001&\u0001&\u0001"+
		"\'\u0001\'\u0001\'\u0001\'\u0001\'\u0001\'\u0001(\u0001(\u0001(\u0001"+
		")\u0001)\u0001)\u0001*\u0001*\u0001*\u0001*\u0001*\u0001+\u0001+\u0001"+
		"+\u0001+\u0001+\u0001+\u0001,\u0001,\u0001,\u0001,\u0001-\u0001-\u0001"+
		"-\u0001.\u0001.\u0001.\u0001.\u0001/\u0001/\u0001/\u0001/\u0001/\u0001"+
		"0\u00010\u00050\u010c\b0\n0\f0\u010f\t0\u00011\u00041\u0112\b1\u000b1"+
		"\f1\u0113\u00011\u00011\u00012\u00012\u00012\u00012\u00052\u011c\b2\n"+
		"2\f2\u011f\t2\u00012\u00012\u00012\u00012\u00012\u00013\u00013\u00013"+
		"\u00013\u00053\u012a\b3\n3\f3\u012d\t3\u00013\u00013\u0001\u011d\u0000"+
		"4\u0001\u0001\u0003\u0002\u0005\u0003\u0007\u0004\t\u0005\u000b\u0006"+
		"\r\u0007\u000f\b\u0011\t\u0013\n\u0015\u000b\u0017\f\u0019\r\u001b\u000e"+
		"\u001d\u000f\u001f\u0010!\u0011#\u0012%\u0013\'\u0014)\u0015+\u0016-\u0017"+
		"/\u00181\u00193\u001a5\u001b7\u001c9\u001d;\u001e=\u001f? A!C\"E#G$I%"+
		"K&M\'O(Q)S*U+W,Y-[.]/_0a1c2e3g4\u0001\u0000\u0005\u0001\u000009\u0003"+
		"\u0000AZ__az\u0004\u000009AZ__az\u0003\u0000\t\n\r\r  \u0002\u0000\n\n"+
		"\r\r\u0134\u0000\u0001\u0001\u0000\u0000\u0000\u0000\u0003\u0001\u0000"+
		"\u0000\u0000\u0000\u0005\u0001\u0000\u0000\u0000\u0000\u0007\u0001\u0000"+
		"\u0000\u0000\u0000\t\u0001\u0000\u0000\u0000\u0000\u000b\u0001\u0000\u0000"+
		"\u0000\u0000\r\u0001\u0000\u0000\u0000\u0000\u000f\u0001\u0000\u0000\u0000"+
		"\u0000\u0011\u0001\u0000\u0000\u0000\u0000\u0013\u0001\u0000\u0000\u0000"+
		"\u0000\u0015\u0001\u0000\u0000\u0000\u0000\u0017\u0001\u0000\u0000\u0000"+
		"\u0000\u0019\u0001\u0000\u0000\u0000\u0000\u001b\u0001\u0000\u0000\u0000"+
		"\u0000\u001d\u0001\u0000\u0000\u0000\u0000\u001f\u0001\u0000\u0000\u0000"+
		"\u0000!\u0001\u0000\u0000\u0000\u0000#\u0001\u0000\u0000\u0000\u0000%"+
		"\u0001\u0000\u0000\u0000\u0000\'\u0001\u0000\u0000\u0000\u0000)\u0001"+
		"\u0000\u0000\u0000\u0000+\u0001\u0000\u0000\u0000\u0000-\u0001\u0000\u0000"+
		"\u0000\u0000/\u0001\u0000\u0000\u0000\u00001\u0001\u0000\u0000\u0000\u0000"+
		"3\u0001\u0000\u0000\u0000\u00005\u0001\u0000\u0000\u0000\u00007\u0001"+
		"\u0000\u0000\u0000\u00009\u0001\u0000\u0000\u0000\u0000;\u0001\u0000\u0000"+
		"\u0000\u0000=\u0001\u0000\u0000\u0000\u0000?\u0001\u0000\u0000\u0000\u0000"+
		"A\u0001\u0000\u0000\u0000\u0000C\u0001\u0000\u0000\u0000\u0000E\u0001"+
		"\u0000\u0000\u0000\u0000G\u0001\u0000\u0000\u0000\u0000I\u0001\u0000\u0000"+
		"\u0000\u0000K\u0001\u0000\u0000\u0000\u0000M\u0001\u0000\u0000\u0000\u0000"+
		"O\u0001\u0000\u0000\u0000\u0000Q\u0001\u0000\u0000\u0000\u0000S\u0001"+
		"\u0000\u0000\u0000\u0000U\u0001\u0000\u0000\u0000\u0000W\u0001\u0000\u0000"+
		"\u0000\u0000Y\u0001\u0000\u0000\u0000\u0000[\u0001\u0000\u0000\u0000\u0000"+
		"]\u0001\u0000\u0000\u0000\u0000_\u0001\u0000\u0000\u0000\u0000a\u0001"+
		"\u0000\u0000\u0000\u0000c\u0001\u0000\u0000\u0000\u0000e\u0001\u0000\u0000"+
		"\u0000\u0000g\u0001\u0000\u0000\u0000\u0001i\u0001\u0000\u0000\u0000\u0003"+
		"k\u0001\u0000\u0000\u0000\u0005m\u0001\u0000\u0000\u0000\u0007o\u0001"+
		"\u0000\u0000\u0000\tq\u0001\u0000\u0000\u0000\u000bs\u0001\u0000\u0000"+
		"\u0000\ru\u0001\u0000\u0000\u0000\u000fw\u0001\u0000\u0000\u0000\u0011"+
		"y\u0001\u0000\u0000\u0000\u0013{\u0001\u0000\u0000\u0000\u0015}\u0001"+
		"\u0000\u0000\u0000\u0017\u007f\u0001\u0000\u0000\u0000\u0019\u0081\u0001"+
		"\u0000\u0000\u0000\u001b\u0083\u0001\u0000\u0000\u0000\u001d\u0085\u0001"+
		"\u0000\u0000\u0000\u001f\u0088\u0001\u0000\u0000\u0000!\u008a\u0001\u0000"+
		"\u0000\u0000#\u008c\u0001\u0000\u0000\u0000%\u008e\u0001\u0000\u0000\u0000"+
		"\'\u0091\u0001\u0000\u0000\u0000)\u0093\u0001\u0000\u0000\u0000+\u0096"+
		"\u0001\u0000\u0000\u0000-\u0098\u0001\u0000\u0000\u0000/\u009b\u0001\u0000"+
		"\u0000\u00001\u009d\u0001\u0000\u0000\u00003\u00a0\u0001\u0000\u0000\u0000"+
		"5\u00a2\u0001\u0000\u0000\u00007\u00a5\u0001\u0000\u0000\u00009\u00a9"+
		"\u0001\u0000\u0000\u0000;\u00ad\u0001\u0000\u0000\u0000=\u00b3\u0001\u0000"+
		"\u0000\u0000?\u00b9\u0001\u0000\u0000\u0000A\u00bf\u0001\u0000\u0000\u0000"+
		"C\u00c2\u0001\u0000\u0000\u0000E\u00c6\u0001\u0000\u0000\u0000G\u00cb"+
		"\u0001\u0000\u0000\u0000I\u00cf\u0001\u0000\u0000\u0000K\u00d6\u0001\u0000"+
		"\u0000\u0000M\u00db\u0001\u0000\u0000\u0000O\u00e2\u0001\u0000\u0000\u0000"+
		"Q\u00e8\u0001\u0000\u0000\u0000S\u00eb\u0001\u0000\u0000\u0000U\u00ee"+
		"\u0001\u0000\u0000\u0000W\u00f3\u0001\u0000\u0000\u0000Y\u00f9\u0001\u0000"+
		"\u0000\u0000[\u00fd\u0001\u0000\u0000\u0000]\u0100\u0001\u0000\u0000\u0000"+
		"_\u0104\u0001\u0000\u0000\u0000a\u0109\u0001\u0000\u0000\u0000c\u0111"+
		"\u0001\u0000\u0000\u0000e\u0117\u0001\u0000\u0000\u0000g\u0125\u0001\u0000"+
		"\u0000\u0000ij\u0005(\u0000\u0000j\u0002\u0001\u0000\u0000\u0000kl\u0005"+
		",\u0000\u0000l\u0004\u0001\u0000\u0000\u0000mn\u0005)\u0000\u0000n\u0006"+
		"\u0001\u0000\u0000\u0000op\u0005{\u0000\u0000p\b\u0001\u0000\u0000\u0000"+
		"qr\u0005}\u0000\u0000r\n\u0001\u0000\u0000\u0000st\u0005;\u0000\u0000"+
		"t\f\u0001\u0000\u0000\u0000uv\u0005[\u0000\u0000v\u000e\u0001\u0000\u0000"+
		"\u0000wx\u0005]\u0000\u0000x\u0010\u0001\u0000\u0000\u0000yz\u0005.\u0000"+
		"\u0000z\u0012\u0001\u0000\u0000\u0000{|\u0005#\u0000\u0000|\u0014\u0001"+
		"\u0000\u0000\u0000}~\u0005&\u0000\u0000~\u0016\u0001\u0000\u0000\u0000"+
		"\u007f\u0080\u0005?\u0000\u0000\u0080\u0018\u0001\u0000\u0000\u0000\u0081"+
		"\u0082\u0005:\u0000\u0000\u0082\u001a\u0001\u0000\u0000\u0000\u0083\u0084"+
		"\u0005=\u0000\u0000\u0084\u001c\u0001\u0000\u0000\u0000\u0085\u0086\u0005"+
		".\u0000\u0000\u0086\u0087\u0005.\u0000\u0000\u0087\u001e\u0001\u0000\u0000"+
		"\u0000\u0088\u0089\u0005*\u0000\u0000\u0089 \u0001\u0000\u0000\u0000\u008a"+
		"\u008b\u0005/\u0000\u0000\u008b\"\u0001\u0000\u0000\u0000\u008c\u008d"+
		"\u0005%\u0000\u0000\u008d$\u0001\u0000\u0000\u0000\u008e\u008f\u0005+"+
		"\u0000\u0000\u008f\u0090\u0005+\u0000\u0000\u0090&\u0001\u0000\u0000\u0000"+
		"\u0091\u0092\u0005+\u0000\u0000\u0092(\u0001\u0000\u0000\u0000\u0093\u0094"+
		"\u0005-\u0000\u0000\u0094\u0095\u0005-\u0000\u0000\u0095*\u0001\u0000"+
		"\u0000\u0000\u0096\u0097\u0005-\u0000\u0000\u0097,\u0001\u0000\u0000\u0000"+
		"\u0098\u0099\u0005>\u0000\u0000\u0099\u009a\u0005=\u0000\u0000\u009a."+
		"\u0001\u0000\u0000\u0000\u009b\u009c\u0005>\u0000\u0000\u009c0\u0001\u0000"+
		"\u0000\u0000\u009d\u009e\u0005<\u0000\u0000\u009e\u009f\u0005=\u0000\u0000"+
		"\u009f2\u0001\u0000\u0000\u0000\u00a0\u00a1\u0005<\u0000\u0000\u00a14"+
		"\u0001\u0000\u0000\u0000\u00a2\u00a3\u0005=\u0000\u0000\u00a3\u00a4\u0005"+
		"=\u0000\u0000\u00a46\u0001\u0000\u0000\u0000\u00a5\u00a6\u0005!\u0000"+
		"\u0000\u00a6\u00a7\u0005=\u0000\u0000\u00a78\u0001\u0000\u0000\u0000\u00a8"+
		"\u00aa\u0007\u0000\u0000\u0000\u00a9\u00a8\u0001\u0000\u0000\u0000\u00aa"+
		"\u00ab\u0001\u0000\u0000\u0000\u00ab\u00a9\u0001\u0000\u0000\u0000\u00ab"+
		"\u00ac\u0001\u0000\u0000\u0000\u00ac:\u0001\u0000\u0000\u0000\u00ad\u00ae"+
		"\u0005a\u0000\u0000\u00ae\u00af\u0005l\u0000\u0000\u00af\u00b0\u0005l"+
		"\u0000\u0000\u00b0\u00b1\u0005o\u0000\u0000\u00b1\u00b2\u0005c\u0000\u0000"+
		"\u00b2<\u0001\u0000\u0000\u0000\u00b3\u00b4\u0005i\u0000\u0000\u00b4\u00b5"+
		"\u0005n\u0000\u0000\u00b5\u00b6\u0005p\u0000\u0000\u00b6\u00b7\u0005u"+
		"\u0000\u0000\u00b7\u00b8\u0005t\u0000\u0000\u00b8>\u0001\u0000\u0000\u0000"+
		"\u00b9\u00ba\u0005w\u0000\u0000\u00ba\u00bb\u0005h\u0000\u0000\u00bb\u00bc"+
		"\u0005i\u0000\u0000\u00bc\u00bd\u0005l\u0000\u0000\u00bd\u00be\u0005e"+
		"\u0000\u0000\u00be@\u0001\u0000\u0000\u0000\u00bf\u00c0\u0005i\u0000\u0000"+
		"\u00c0\u00c1\u0005f\u0000\u0000\u00c1B\u0001\u0000\u0000\u0000\u00c2\u00c3"+
		"\u0005f\u0000\u0000\u00c3\u00c4\u0005o\u0000\u0000\u00c4\u00c5\u0005r"+
		"\u0000\u0000\u00c5D\u0001\u0000\u0000\u0000\u00c6\u00c7\u0005e\u0000\u0000"+
		"\u00c7\u00c8\u0005l\u0000\u0000\u00c8\u00c9\u0005s\u0000\u0000\u00c9\u00ca"+
		"\u0005e\u0000\u0000\u00caF\u0001\u0000\u0000\u0000\u00cb\u00cc\u0005v"+
		"\u0000\u0000\u00cc\u00cd\u0005a\u0000\u0000\u00cd\u00ce\u0005r\u0000\u0000"+
		"\u00ceH\u0001\u0000\u0000\u0000\u00cf\u00d0\u0005r\u0000\u0000\u00d0\u00d1"+
		"\u0005e\u0000\u0000\u00d1\u00d2\u0005t\u0000\u0000\u00d2\u00d3\u0005u"+
		"\u0000\u0000\u00d3\u00d4\u0005r\u0000\u0000\u00d4\u00d5\u0005n\u0000\u0000"+
		"\u00d5J\u0001\u0000\u0000\u0000\u00d6\u00d7\u0005n\u0000\u0000\u00d7\u00d8"+
		"\u0005u\u0000\u0000\u00d8\u00d9\u0005l\u0000\u0000\u00d9\u00da\u0005l"+
		"\u0000\u0000\u00daL\u0001\u0000\u0000\u0000\u00db\u00dc\u0005o\u0000\u0000"+
		"\u00dc\u00dd\u0005u\u0000\u0000\u00dd\u00de\u0005t\u0000\u0000\u00de\u00df"+
		"\u0005p\u0000\u0000\u00df\u00e0\u0005u\u0000\u0000\u00e0\u00e1\u0005t"+
		"\u0000\u0000\u00e1N\u0001\u0000\u0000\u0000\u00e2\u00e3\u0005e\u0000\u0000"+
		"\u00e3\u00e4\u0005r\u0000\u0000\u00e4\u00e5\u0005r\u0000\u0000\u00e5\u00e6"+
		"\u0005o\u0000\u0000\u00e6\u00e7\u0005r\u0000\u0000\u00e7P\u0001\u0000"+
		"\u0000\u0000\u00e8\u00e9\u0005o\u0000\u0000\u00e9\u00ea\u0005f\u0000\u0000"+
		"\u00eaR\u0001\u0000\u0000\u0000\u00eb\u00ec\u0005b\u0000\u0000\u00ec\u00ed"+
		"\u0005y\u0000\u0000\u00edT\u0001\u0000\u0000\u0000\u00ee\u00ef\u0005t"+
		"\u0000\u0000\u00ef\u00f0\u0005r\u0000\u0000\u00f0\u00f1\u0005u\u0000\u0000"+
		"\u00f1\u00f2\u0005e\u0000\u0000\u00f2V\u0001\u0000\u0000\u0000\u00f3\u00f4"+
		"\u0005f\u0000\u0000\u00f4\u00f5\u0005a\u0000\u0000\u00f5\u00f6\u0005l"+
		"\u0000\u0000\u00f6\u00f7\u0005s\u0000\u0000\u00f7\u00f8\u0005e\u0000\u0000"+
		"\u00f8X\u0001\u0000\u0000\u0000\u00f9\u00fa\u0005a\u0000\u0000\u00fa\u00fb"+
		"\u0005n\u0000\u0000\u00fb\u00fc\u0005d\u0000\u0000\u00fcZ\u0001\u0000"+
		"\u0000\u0000\u00fd\u00fe\u0005o\u0000\u0000\u00fe\u00ff\u0005r\u0000\u0000"+
		"\u00ff\\\u0001\u0000\u0000\u0000\u0100\u0101\u0005n\u0000\u0000\u0101"+
		"\u0102\u0005o\u0000\u0000\u0102\u0103\u0005t\u0000\u0000\u0103^\u0001"+
		"\u0000\u0000\u0000\u0104\u0105\u0005p\u0000\u0000\u0105\u0106\u0005o\u0000"+
		"\u0000\u0106\u0107\u0005l\u0000\u0000\u0107\u0108\u0005y\u0000\u0000\u0108"+
		"`\u0001\u0000\u0000\u0000\u0109\u010d\u0007\u0001\u0000\u0000\u010a\u010c"+
		"\u0007\u0002\u0000\u0000\u010b\u010a\u0001\u0000\u0000\u0000\u010c\u010f"+
		"\u0001\u0000\u0000\u0000\u010d\u010b\u0001\u0000\u0000\u0000\u010d\u010e"+
		"\u0001\u0000\u0000\u0000\u010eb\u0001\u0000\u0000\u0000\u010f\u010d\u0001"+
		"\u0000\u0000\u0000\u0110\u0112\u0007\u0003\u0000\u0000\u0111\u0110\u0001"+
		"\u0000\u0000\u0000\u0112\u0113\u0001\u0000\u0000\u0000\u0113\u0111\u0001"+
		"\u0000\u0000\u0000\u0113\u0114\u0001\u0000\u0000\u0000\u0114\u0115\u0001"+
		"\u0000\u0000\u0000\u0115\u0116\u00061\u0000\u0000\u0116d\u0001\u0000\u0000"+
		"\u0000\u0117\u0118\u0005/\u0000\u0000\u0118\u0119\u0005*\u0000\u0000\u0119"+
		"\u011d\u0001\u0000\u0000\u0000\u011a\u011c\t\u0000\u0000\u0000\u011b\u011a"+
		"\u0001\u0000\u0000\u0000\u011c\u011f\u0001\u0000\u0000\u0000\u011d\u011e"+
		"\u0001\u0000\u0000\u0000\u011d\u011b\u0001\u0000\u0000\u0000\u011e\u0120"+
		"\u0001\u0000\u0000\u0000\u011f\u011d\u0001\u0000\u0000\u0000\u0120\u0121"+
		"\u0005*\u0000\u0000\u0121\u0122\u0005/\u0000\u0000\u0122\u0123\u0001\u0000"+
		"\u0000\u0000\u0123\u0124\u00062\u0000\u0000\u0124f\u0001\u0000\u0000\u0000"+
		"\u0125\u0126\u0005/\u0000\u0000\u0126\u0127\u0005/\u0000\u0000\u0127\u012b"+
		"\u0001\u0000\u0000\u0000\u0128\u012a\b\u0004\u0000\u0000\u0129\u0128\u0001"+
		"\u0000\u0000\u0000\u012a\u012d\u0001\u0000\u0000\u0000\u012b\u0129\u0001"+
		"\u0000\u0000\u0000\u012b\u012c\u0001\u0000\u0000\u0000\u012c\u012e\u0001"+
		"\u0000\u0000\u0000\u012d\u012b\u0001\u0000\u0000\u0000\u012e\u012f\u0006"+
		"3\u0000\u0000\u012fh\u0001\u0000\u0000\u0000\u0006\u0000\u00ab\u010d\u0113"+
		"\u011d\u012b\u0001\u0006\u0000\u0000";
	public static final ATN _ATN =
		new ATNDeserializer().deserialize(_serializedATN.toCharArray());
	static {
		_decisionToDFA = new DFA[_ATN.getNumberOfDecisions()];
		for (int i = 0; i < _ATN.getNumberOfDecisions(); i++) {
			_decisionToDFA[i] = new DFA(_ATN.getDecisionState(i), i);
		}
	}
}