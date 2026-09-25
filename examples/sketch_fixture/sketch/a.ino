/* SPDX-License-Identifier: Apache-2.0 */
int sequence_middle = sequence * 10 + 2;
void drawLater()
{
    int value = worker_value();
    printk("[sketch_fixture] order=%d worker=%d resource=%d\n", sequence_end, value, worker_resource());
    board.clear(); board.setCursor(0, 0);
    board.print(sequence_end == 123 ? "Sketch order PASS\n" : "Sketch order FAIL\n");
    board.print(value); board.print(" / "); board.print(worker_resource()); board.display();
}
