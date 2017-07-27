#include <stdlib.h>
#include <string.h>
#include "coroutine_impl.h"
#include "coroutine_task.h"

static void *task_main(void *arg) {
  task_t *task = (task_t*)arg;

  while (1) {
    if (task->state == TASK_WAITING) {
      coroutine_yield_context();
      continue;
    }

    if (task->state == TASK_END) {
      break;
    }

    task->pool->working++;
    task->coroutine->fun(task->arg); 

    // return to free list
    task->next = task->pool->free;
    task->pool->free = task;
    task->pool->working--;

    // reset param
    task->arg = NULL;
    task->coroutine->fun = NULL;
    task->state = TASK_WAITING;
  }

  return NULL;
}

void new_task(taskpool_t *pool, coroutine_fun_t fun, void *arg) {
  if (pool->working == pool->size) {
    // TODO
  }

  task_t *task = pool->free;
  if (task == NULL) {
    // TODO
  }

  pool->free = task->next;
  task->next = NULL;

  task->arg = arg;
  task->coroutine->fun = fun;
  task->state = TASK_RUNNING;
  coroutine_resume(task->coroutine);
}

taskpool_t* create_thread_taskpool(env_t *env, int size) {
  taskpool_t *pool = (taskpool_t*)malloc(sizeof(taskpool_t));
  memset(pool, 0, sizeof(taskpool_t));
  pool->tasks = (task_t**)malloc(sizeof(task_t*));

  for (int i = 0;i < size; ++i) {
    task_t *task = (task_t*)malloc(sizeof(task_t));
    memset(task, 0, sizeof(task_t));
    pool->tasks[i] = task;
    task->pool = pool;
    coroutine_t *co = coroutine_new(task_main, task);
    task->coroutine = co;
    task->next = pool->free;
    task->state = TASK_WAITING;
    pool->free = task;
    coroutine_resume(co);
  }
  pool->env = env;
  pool->size = size;

  return pool;
}