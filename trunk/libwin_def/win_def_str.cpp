#include "win_def.h"

PWSTR				CharFirstOf(PCWSTR in, PCWSTR mask) {
	size_t	lin = Len(in);
	size_t	len = Len(mask);
	for (size_t i = 0; i < lin; ++i) {
		for (size_t j = 0; j < len; ++j) {
			if (in[i] == mask[j])
				return	(PWSTR)&in[i];
		}
	}
	return	NULL;
}
PWSTR				CharFirstNotOf(PCWSTR in, PCWSTR mask) {
	size_t	lin = Len(in);
	size_t	len = Len(mask);
	for (size_t i = 0; i < lin; ++i) {
		for (size_t j = 0; j < len; ++j) {
			if (in[i] == mask[j])
				break;
			if (j == len - 1)
				return	(PWSTR)&in[i];
		}
	}
	return	NULL;
}
PWSTR				CharLastOf(PCWSTR in, PCWSTR mask) {
	size_t	len = Len(mask);
	for (size_t i = Len(in); i > 0; --i) {
		for (size_t j = 0; j < len; ++j) {
			if (in[i-1] == mask[j])
				return	(PWSTR)&in[i-1];
		}
	}
	return	NULL;
}
PWSTR				CharLastNotOf(PCWSTR in, PCWSTR mask) {
	size_t	len = Len(mask);
	for (size_t i = Len(in); i > 0; --i) {
		for (size_t j = 0; j < len; ++j) {
			if (in[i-1] == mask[j])
				break;
			if (j == len - 1)
				return	(PWSTR)&in[i-1];
		}
	}
	return	NULL;
}
