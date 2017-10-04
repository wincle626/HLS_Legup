unsigned getAccel(int *count, unsigned *IDList)
{
    unsigned ID = IDList[*count];
    (*count)++;
    return ID;
}

void freeAccel(int *count, unsigned id, unsigned *IDList)
{
    (*count)--;
    IDList[*count] = id;
}
