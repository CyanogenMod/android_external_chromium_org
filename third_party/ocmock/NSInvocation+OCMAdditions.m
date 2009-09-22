//---------------------------------------------------------------------------------------
//  $Id: NSInvocation+OCMAdditions.m 50 2009-07-16 06:48:19Z erik $
//  Copyright (c) 2006-2009 by Mulle Kybernetik. See License file for details.
//---------------------------------------------------------------------------------------

#import "NSInvocation+OCMAdditions.h"


@implementation NSInvocation(OCMAdditions)

- (id)getArgumentAtIndexAsObject:(int)index
{
	const char* argType;
	
	argType = [[self methodSignature] getArgumentTypeAtIndex:index];
	while(strchr("rnNoORV", argType[0]) != NULL)
		argType += 1;
	
	if((strlen(argType) > 1) && (strchr("{^", argType[0]) == NULL))
		[NSException raise:NSInvalidArgumentException format:@"Cannot handle argument type '%s'.", argType];
	
	switch (argType[0]) 
	{
		case '#':
		case '@': 
		{
			id value;
			[self getArgument:&value atIndex:index];
			return value;
		}
		case ':':
 		{
 			SEL s = (SEL)0;
 			[self getArgument:&s atIndex:index];
 			id value = NSStringFromSelector(s);
 			return value;
 		}
		case 'i': 
		{
			int value;
			[self getArgument:&value atIndex:index];
			return [NSNumber numberWithInt:value];
		}	
		case 's':
		{
			short value;
			[self getArgument:&value atIndex:index];
			return [NSNumber numberWithShort:value];
		}	
		case 'l':
		{
			long value;
			[self getArgument:&value atIndex:index];
			return [NSNumber numberWithLong:value];
		}	
		case 'q':
		{
			long long value;
			[self getArgument:&value atIndex:index];
			return [NSNumber numberWithLongLong:value];
		}	
		case 'c':
		{
			char value;
			[self getArgument:&value atIndex:index];
			return [NSNumber numberWithChar:value];
		}	
		case 'C':
		{
			unsigned char value;
			[self getArgument:&value atIndex:index];
			return [NSNumber numberWithUnsignedChar:value];
		}	
		case 'I':
		{
			unsigned int value;
			[self getArgument:&value atIndex:index];
			return [NSNumber numberWithUnsignedInt:value];
		}	
		case 'S':
		{
			unsigned short value;
			[self getArgument:&value atIndex:index];
			return [NSNumber numberWithUnsignedShort:value];
		}	
		case 'L':
		{
			unsigned long value;
			[self getArgument:&value atIndex:index];
			return [NSNumber numberWithUnsignedLong:value];
		}	
		case 'Q':
		{
			unsigned long long value;
			[self getArgument:&value atIndex:index];
			return [NSNumber numberWithUnsignedLongLong:value];
		}	
		case 'f':
		{
			float value;
			[self getArgument:&value atIndex:index];
			return [NSNumber numberWithFloat:value];
		}	
		case 'd':
		{
			double value;
			[self getArgument:&value atIndex:index];
			return [NSNumber numberWithDouble:value];
		}	
		case 'B':
		{
			bool value;
			[self getArgument:&value atIndex:index];
			return [NSNumber numberWithBool:value];
		}
		case '^':
        {
            void *value = NULL;
            [self getArgument:&value atIndex:index];
            return [NSValue valueWithPointer:value];
        }
		case '{': // structure
		{
			unsigned maxSize = [[self methodSignature] frameLength];
			NSMutableData *argumentData = [[[NSMutableData alloc] initWithLength:maxSize] autorelease];
			[self getArgument:[argumentData mutableBytes] atIndex:index];
			return [NSValue valueWithBytes:[argumentData bytes] objCType:argType];
		}       
			
	}
	[NSException raise:NSInvalidArgumentException format:@"Argument type '%s' not supported", argType];
	return nil;
}

- (NSString *)invocationDescription
{
	NSMethodSignature *methodSignature = [self methodSignature];
	unsigned int numberOfArgs = [methodSignature numberOfArguments];
	
	if (numberOfArgs == 2)
		return NSStringFromSelector([self selector]);
	
	NSArray *selectorParts = [NSStringFromSelector([self selector]) componentsSeparatedByString:@":"];
	NSMutableString *description = [[NSMutableString alloc] init];
	int i;
	for(i = 2; i < numberOfArgs; i++)
	{
		[description appendFormat:@"%@%@:", (i > 2 ? @" " : @""), [selectorParts objectAtIndex:(i - 2)]];
		[description appendString:[self argumentDescriptionAtIndex:i]];
	}
	
	return [description autorelease];
}

- (NSString *)argumentDescriptionAtIndex:(int)index
{
	const char *argType = [[self methodSignature] getArgumentTypeAtIndex:index];
	if(strchr("rnNoORV", argType[0]) != NULL)
		argType += 1;

	switch(*argType)
	{
		case '@':	return [self objectDescriptionAtIndex:index];
		case 'c':	return [self charDescriptionAtIndex:index];
		case 'C':	return [self unsignedCharDescriptionAtIndex:index];
		case 'i':	return [self intDescriptionAtIndex:index];
		case 'I':	return [self unsignedIntDescriptionAtIndex:index];
		case 's':	return [self shortDescriptionAtIndex:index];
		case 'S':	return [self unsignedShortDescriptionAtIndex:index];
		case 'l':	return [self longDescriptionAtIndex:index];
		case 'L':	return [self unsignedLongDescriptionAtIndex:index];
		case 'q':	return [self longLongDescriptionAtIndex:index];
		case 'Q':	return [self unsignedLongLongDescriptionAtIndex:index];
		case 'd':	return [self doubleDescriptionAtIndex:index];
		case 'f':	return [self floatDescriptionAtIndex:index];
		// Why does this throw EXC_BAD_ACCESS when appending the string?
		//	case NSObjCStructType: return [self structDescriptionAtIndex:index];
		case '^':	return [self pointerDescriptionAtIndex:index];
		case '*':	return [self cStringDescriptionAtIndex:index];
		case ':':	return [self selectorDescriptionAtIndex:index];
		default:	return [@"<??" stringByAppendingString:@">"];  // avoid confusion with trigraphs...
	}
	
}


- (NSString *)objectDescriptionAtIndex:(int)anInt
{
	id object;
	
	[self getArgument:&object atIndex:anInt];
	if (object == nil)
		return @"nil";
	else if([object isKindOfClass:[NSString class]])
		return [NSString stringWithFormat:@"@\"%@\"", [object description]];
	else
		return [object description];
}

- (NSString *)charDescriptionAtIndex:(int)anInt
{
	unsigned char buffer[128];
	memset(buffer, 0x0, 128);
	
	[self getArgument:&buffer atIndex:anInt];
	
	// If there's only one character in the buffer, and it's 0 or 1, then we have a BOOL
	if (buffer[1] == '\0' && (buffer[0] == 0 || buffer[0] == 1))
		return [NSString stringWithFormat:@"%@", (buffer[0] == 1 ? @"YES" : @"NO")];
	else
		return [NSString stringWithFormat:@"'%c'", *buffer];
}

- (NSString *)unsignedCharDescriptionAtIndex:(int)anInt
{
	unsigned char buffer[128];
	memset(buffer, 0x0, 128);
	
	[self getArgument:&buffer atIndex:anInt];
	return [NSString stringWithFormat:@"'%c'", *buffer];
}

- (NSString *)intDescriptionAtIndex:(int)anInt
{
	int intValue;
	
	[self getArgument:&intValue atIndex:anInt];
	return [NSString stringWithFormat:@"%d", intValue];
}

- (NSString *)unsignedIntDescriptionAtIndex:(int)anInt
{
	unsigned int intValue;
	
	[self getArgument:&intValue atIndex:anInt];
	return [NSString stringWithFormat:@"%d", intValue];
}

- (NSString *)shortDescriptionAtIndex:(int)anInt
{
	short shortValue;
	
	[self getArgument:&shortValue atIndex:anInt];
	return [NSString stringWithFormat:@"%hi", shortValue];
}

- (NSString *)unsignedShortDescriptionAtIndex:(int)anInt
{
	unsigned short shortValue;
	
	[self getArgument:&shortValue atIndex:anInt];
	return [NSString stringWithFormat:@"%hu", shortValue];
}

- (NSString *)longDescriptionAtIndex:(int)anInt
{
	long longValue;
	
	[self getArgument:&longValue atIndex:anInt];
	return [NSString stringWithFormat:@"%d", longValue];
}

- (NSString *)unsignedLongDescriptionAtIndex:(int)anInt
{
	unsigned long longValue;
	
	[self getArgument:&longValue atIndex:anInt];
	return [NSString stringWithFormat:@"%u", longValue];
}

- (NSString *)longLongDescriptionAtIndex:(int)anInt
{
	long long longLongValue;
	
	[self getArgument:&longLongValue atIndex:anInt];
	return [NSString stringWithFormat:@"%qi", longLongValue];
}

- (NSString *)unsignedLongLongDescriptionAtIndex:(int)anInt
{
	unsigned long long longLongValue;
	
	[self getArgument:&longLongValue atIndex:anInt];
	return [NSString stringWithFormat:@"%qu", longLongValue];
}

- (NSString *)doubleDescriptionAtIndex:(int)anInt;
{
	double doubleValue;
	
	[self getArgument:&doubleValue atIndex:anInt];
	return [NSString stringWithFormat:@"%f", doubleValue];
}

- (NSString *)floatDescriptionAtIndex:(int)anInt
{
	float floatValue;
	
	[self getArgument:&floatValue atIndex:anInt];
	return [NSString stringWithFormat:@"%f", floatValue];
}

- (NSString *)structDescriptionAtIndex:(int)anInt;
{
	void *buffer;
	
	[self getArgument:&buffer atIndex:anInt];
	return [NSString stringWithFormat:@":(struct)%p", buffer];
}

- (NSString *)pointerDescriptionAtIndex:(int)anInt
{
	void *buffer;
	
	[self getArgument:&buffer atIndex:anInt];
	return [NSString stringWithFormat:@"%p", buffer];
}

- (NSString *)cStringDescriptionAtIndex:(int)anInt
{
	char buffer[128];
	
	memset(buffer, 0x0, 128);
	
	[self getArgument:&buffer atIndex:anInt];
	return [NSString stringWithFormat:@"\"%S\"", buffer];
}

- (NSString *)selectorDescriptionAtIndex:(int)anInt
{
	SEL selectorValue;
	
	[self getArgument:&selectorValue atIndex:anInt];
	return [NSString stringWithFormat:@"@selector(%@)", NSStringFromSelector(selectorValue)];
}

@end
