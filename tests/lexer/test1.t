TokenPtr initStrToken(StrTokenPtr tok, LXR_TokenType type, int len){
    initToken((TokenPtr)tok, type);
    tok->buff = malloc(len + 1);
    for(int i = 0; i < len; i++) tok->buff[i] = buff[i];
    tok->buff[len] = 0;
    tok->length = len;
    return (TokenPtr)tok;
}
