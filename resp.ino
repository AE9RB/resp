// Copyright 2015 David Turnbull. All rights reserved.

#define KEY_VALUE_BUF_SIZE (512)
#define COMMAND_BUF_SIZE (128)
#define COMMAND_MAX_COUNT (8)

void cmd_DOUT(uint8_t pin, uint8_t value);
void cmd_SET(char* key, char* value);
void cmd_DEL(char* key);
void cmd_GET(char* key);

char kvBuf[KEY_VALUE_BUF_SIZE];
char commandBuf[COMMAND_BUF_SIZE];
char* commandPtr[COMMAND_MAX_COUNT];
int commandLen[COMMAND_MAX_COUNT];
int state;
int bufPos;
int commandCount;

void initCommandPart() {
  commandPtr[commandCount] = &commandBuf[bufPos];
  commandLen[commandCount] = 0;
}

void initCommand() {
  state = 0;
  bufPos = 0;
  commandCount = 0;
  initCommandPart();
}

void setup() {
  int i;
  for (i=0; i < KEY_VALUE_BUF_SIZE; i++) kvBuf[i]=0;
  Serial.begin(57600);
  initCommand();
}


void doCommand() {
  bool argCountError = false;
  
  if (!strcasecmp("DOUT", commandPtr[0])) {
    if (commandCount != 3) argCountError = true;
    else cmd_DOUT(String(commandPtr[1]).toInt(), String(commandPtr[2]).toInt());
  }
  else if (!strcasecmp("SET", commandPtr[0])) {
    if (commandCount != 3) argCountError = true;
    else cmd_SET(commandPtr[1], commandPtr[2]);
  }
  else if (!strcasecmp("DEL", commandPtr[0])) {
    if (commandCount != 2) argCountError = true;
    else cmd_DEL(commandPtr[1]);
  }
  else if (!strcasecmp("GET", commandPtr[0])) {
    if (commandCount != 2) argCountError = true;
    else cmd_GET(commandPtr[1]);
  }
  else {
    Serial.println("-ERROR unknown command");
    return;
  }
  
  if (argCountError) {
    Serial.println("-ERROR argument count");
  }
}


void loop() {
  int b;
  
  b = Serial.read();
  if (b < 0) return;
  
  if (state == 0) {
    switch (b) {
//      case '+':
//      case '-':
//      case ':':
//      case '$':
//      case '*':
//        state = b;
//        return;
      case '\r':
      case '\n':
        // nop      
        return;
      default:
        if (bufPos != 0) {
          Serial.println("-ERROR command syntax");
          initCommand();
          return;
        }
        state = -1;
        break;
    }
  }
  
  if (state == 0 && bufPos == 0) state = -1;
  
  
  if (state == -1) {
    if (b == '\r' || b == '\n') {
      if (commandCount >= COMMAND_MAX_COUNT || bufPos >= COMMAND_BUF_SIZE) {
        Serial.println("-ERROR command buffer overflow");
        initCommand();
        return;
      }
      commandBuf[bufPos] = 0;
      commandCount++;
      doCommand();
      initCommand();
      return;
    }

    if (b == ' ') {
      if (!bufPos) {
        return;
      }
      if (bufPos < COMMAND_BUF_SIZE) {
        commandBuf[bufPos] = 0;
        bufPos++;
      }
      if (commandCount < COMMAND_MAX_COUNT) {
        commandCount++;
        if (commandCount < COMMAND_MAX_COUNT) {
          initCommandPart();
        }
      }
      return;
    }
    
    if (bufPos < COMMAND_BUF_SIZE && commandCount < COMMAND_MAX_COUNT) {
      commandBuf[bufPos] = b;
      commandLen[commandCount]++;
      bufPos++;
    }
    
    return;
  }
  

}

void cmd_DOUT(uint8_t pin, uint8_t value) {
  pinMode(pin, OUTPUT);
  digitalWrite(pin, value);
  Serial.println("+OK");
}


int deleteKey(char* key) {
  int i = 0, j = 0, k = -1;
  bool onKey = true, matchKey = true;
  
  for (;i < KEY_VALUE_BUF_SIZE; i++) {
    if (k >= 0) {
      kvBuf[k] = kvBuf[i];
      k++;
    }
    if (onKey) {
      if (kvBuf[i] == 0) {
        if (j == 0) break;
        onKey = false;
        continue;
      }
      if (key[j] != kvBuf[i]) matchKey = false;
    } else {
      if (kvBuf[i] == 0) {
        if (matchKey) k = i - j - 1;
        j = 0;
        onKey = true;
        matchKey = true;
        continue;
      }
    }
    j++;
  }
  if (k >= 0) i = k;
  if (i < KEY_VALUE_BUF_SIZE) kvBuf[i] = 0;
  return i;
}


void cmd_SET(char* key, char* value) {
  int i = 0, j = 0, k = 0;
  bool onKey = true, matchKey = true;
  
  for (;i < KEY_VALUE_BUF_SIZE; i++) {
    if (onKey) {
      if (kvBuf[i] == 0) {
        if (j == 0) break;
        onKey = false;
        if (matchKey) k = j + 2; // 2 nulls
        continue;
      }
      if (key[j] != kvBuf[i]) matchKey = false;
      j++;
    } else {
      if (kvBuf[i] == 0) {
        j = 0;
        onKey = true;
        matchKey = true;
        continue;
      }
      if (matchKey) k++;
    }
  }
  
  if (strlen(key) + strlen(value) + 2 > KEY_VALUE_BUF_SIZE - i + k) {
      Serial.println("-ERROR storage overflow");
      return;
  }
  
  i = deleteKey(key);
  strcpy(&kvBuf[i], key);
  i += strlen(key) + 1;
  strcpy(&kvBuf[i], value);
  i += strlen(value) + 1;
  if (i < KEY_VALUE_BUF_SIZE) kvBuf[i] = 0;

  Serial.println("+OK");
}

void cmd_DEL(char* key) {
  deleteKey(key);
  Serial.println("+OK");
}

void cmd_GET(char* key) {
  int i = 0, j = 0;
  bool onKey = true, matchKey = true;
  
  for (;i < KEY_VALUE_BUF_SIZE; i++) {
    if (onKey) {
      if (kvBuf[i] == 0) {
        if (j == 0) break;
        onKey = false;
        if (matchKey) {
          Serial.print("+");
          Serial.println(&kvBuf[i+1]);
          return;
        }
        continue;
      }
      if (key[j] != kvBuf[i]) matchKey = false;
      j++;
    } else {
      if (kvBuf[i] == 0) {
        j = 0;
        onKey = true;
        matchKey = true;
        continue;
      }
    }
  }
  Serial.println("-ERROR not found");
}

