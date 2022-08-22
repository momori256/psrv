int echo();
int echo_pool();
int keep_sending_msg();
int cqueue_test();

int main(int argc, char* argv[]) {
  switch (argv[1][0]) {
    case '0':
      echo();
      break;
    case '1':
      keep_sending_msg();
      break;
    case '2':
      echo_pool();
      break;
    case 't':
      cqueue_test();
      break;
  }
  return 0;
}
