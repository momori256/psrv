int echo();
int keep_sending_msg();

int main(int argc, char *argv[]) {
  switch (argv[1][0]) {
    case '0':
      echo();
      break;
    case '1':
      keep_sending_msg();
      break;
  }
  return 0;
}
