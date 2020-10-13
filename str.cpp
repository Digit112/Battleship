// Just a place to stick some useful functions for handling the command line input.

// Attempts to find a copy of "substr" in "str" and returns the index of the first character in "str" where a match was found.
// The search can be limited to the range str[start] - str[end] (inclusive)
// Both strings are expected to be null-terminated.
int match(char* substr, char* str, int start, int end) {
	int chk = 0;
	for (int i = start; true; i++) {
		if (str[i] == '\0' || i > end) return -1;
		if (substr[chk] == '\0') return i-chk;
		if (str[i] == substr[chk]) chk++;
	}
}

// This is identical to match(). I don't want to delete it because I love it.
// int m(char* s,char* S,int b,int e){int c=0;for(int i=b;1;i++){if(S[i]==0||i>e)return -1;if(s[c]==0)return i-c;if(S[i]==s[c])c++;}}

// Converts all uppercase characters in the string to lowercase characters
char* lowercase(char* str) {
	for (int i = 0; true; i++) {
		if (str[i] == '\0') return str;
		
		if (str[i] >= 65 && str[i] <= 90) {
			str[i] = str[i] | 0x20;
		}
	}
}

// Removes leading and trailing spaces, tabs, and newlines from a string
char* trim(char* str) {
	int fc = -1;
	int lc = -1;
	
	// Find first char
	for (int i = 0; true; i++) {
		if (str[i] == '\0') break;
		
		if (str[i] != ' ' && str[i] != '\t' && str[i] != '\n') {
			fc = i;
			break;
		}
	}
	
	// If there is none and it is all whitespace, return an empty string.
	if (fc == -1) {
		str[0] = '\0';
		return str;
	}
	
	// Find last char
	lc = fc;
	for (int i = fc+1; true; i++) {
		if (str[i] == '\0') break;
		
		if (str[i] != ' ' && str[i] != '\t' && str[i] != '\n') {
			lc = i;
		}
	}
	
	int o = 0;
	for (int n = fc; n <= lc; n++, o++) {
		str[o] = str[n];
	}
	str[o] = '\0';
	
	return str;
}

// Replaces all sequences of whitespace characters with a single space
char* clean_ws(char* str) {
	bool is_in_ws = false;
	int o = 0;
	
	for (int i = 0; true; i++) {
		if (str[i] == '\0') {
			str[o] = '\0';
			return str;
		}
		
		if (str[i] != ' ' && str[i] != '\t' && str[i] != '\n') {
			is_in_ws = false;
			str[o] = str[i];
			o++;
		} else {
			if (is_in_ws) {
				continue;
			} else {
				is_in_ws = true;
				str[o] = ' ';
				o++;
			}
		}
	}
}

// Replace all spaces with null-terminating characters and return a pointer to each new string created by this process. The result is returned to argret, the function returns the number of args found.
// Will return after maxargs arguments are found or a null terminating character is found.
int toargs(char* str, char** argret, int maxargs) {
	if (maxargs == 0) return 0;
	
	int argretn = 1;
	argret[0] = str;
	
	for (int i = 0; true; i++) {
		if (str[i] == '\0') return argretn;
		
		if (str[i] == ' ') {
			str[i] = '\0';
			
			if (argretn == maxargs) {
				return argretn;
			}
			
			argret[argretn] = str+i+1;
			argretn++;
		}
	}
}

// Returns if these strings are identical in contents.
bool str_equ(char* a, char* b) {
	for (int i = 0; true; i++) {
		if (a[i] != b[i]) return false;
		if (a[i] == '\0') return true;
	}
}

