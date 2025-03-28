



// 线程池（Thread Pool）：
// 当请求大量涌入时，动态创建/销毁线程的开销很大（包含系统调用和调度开销）
// 使用线程池可以在启动服务器时就创建固定数量的工作线程，将请求任务分配给这些线程执行
// 而不是每次请求来临时创建线程。这样减少了创建线程的成本、上下文切换的频率，
// 并提高服务器的并发处理效率。