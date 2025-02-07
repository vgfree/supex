/**  
 * Copyright (c) 2009 Carnegie Mellon University. 
 *     All rights reserved.
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing,
 *  software distributed under the License is distributed on an "AS
 *  IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either
 *  express or implied.  See the License for the specific language
 *  governing permissions and limitations under the License.
 *
 * For more about this software visit:
 *
 *      http://www.graphlab.ml.cmu.edu
 *
 */


/**
 * \author jegonzal This class defines a multiqueue version of the
 * priority scheduler.
 **/
#ifndef GRAPHLAB_MULTIQUEUE_PRIORITY_SCHEDULER_HPP
#define GRAPHLAB_MULTIQUEUE_PRIORITY_SCHEDULER_HPP

#include <queue>
#include <cmath>
#include <cassert>

#include <graphlab/tasks/update_task.hpp>

#include <graphlab/util/mutable_queue.hpp>
#include <graphlab/graph/graph.hpp>
#include <graphlab/scope/iscope.hpp>

#include <graphlab/schedulers/ischeduler.hpp>
#include <graphlab/parallel/pthread_tools.hpp>
#include <graphlab/schedulers/support/direct_callback.hpp>
#include <graphlab/schedulers/support/binary_vertex_task_set.hpp>

#include <graphlab/util/task_count_termination.hpp>



#include <graphlab/macros_def.hpp>
namespace graphlab {

   /** \ingroup group_schedulers
    */
  template<typename Graph>
  class multiqueue_priority_scheduler : 
    public ischeduler<Graph> {
  
  public:
    typedef Graph graph_type;
    typedef ischeduler<Graph> base;

    typedef typename base::vertex_id_type vertex_id_type;
    typedef typename base::iengine_type iengine_type;
    typedef typename base::update_task_type     update_task_type;
    typedef typename base::update_function_type update_function_type;
    typedef typename base::callback_type        callback_type;
    typedef typename base::monitor_type         monitor_type;
    typedef task_count_termination terminator_type;
    
    typedef mutable_queue<update_task_type, double> taskqueue_type;

  private:
    using base::monitor;

  public:

    multiqueue_priority_scheduler(iengine_type* engine,
                                  Graph& g, 
                                  size_t ncpus) : 
      callbacks(ncpus, direct_callback<Graph>(this, engine)), 
      binary_vertex_tasks(g.local_vertices()) {
      numvertices = g.local_vertices();
        
      /* How many queues per cpu. More queues, less contention */
      queues_per_cpu = 1;
      num_queues = queues_per_cpu * ncpus;
       
      /* Each cpu keeps record of the queue it last 
         used to keep balance */
      lastqueue.resize(ncpus, 0);
       
      // Do this in the preconstructor
      task_queues.resize(num_queues);
      queue_locks.resize(num_queues);
    }

  
    ~multiqueue_priority_scheduler() {}

    callback_type& get_callback(size_t cpuid) {
      return callbacks[cpuid];
    }

    void start() {};
    
    /** Get the next element in the queue */
    sched_status::status_enum get_next_task(size_t cpuid,
                                            update_task_type &ret_task) {
      bool found = false;
      /* First check my own queues. Keep track which own queue was checked
         so next time I can check next of my own queues to keep balance. */
      size_t firstown = cpuid * queues_per_cpu;
      for(size_t ownq_i = 0; ownq_i < queues_per_cpu; ++ownq_i) {
        size_t queueidx = 
          firstown + ((ownq_i + lastqueue[cpuid] + 1) % queues_per_cpu);
        taskqueue_type& queue = task_queues[queueidx];
        queue_locks[queueidx].lock();
        if (!queue.empty()) {
          ret_task = queue.pop().first;
          found = true;
          binary_vertex_tasks.remove(ret_task);
          lastqueue[cpuid] = ownq_i;
        }
        queue_locks[queueidx].unlock();
        if (found) break;
      }
  
      /* Ok, my queues were empty - now check every other queue */
      if (!found) {
        /* First check own queue - if it is empty, check others */
        for(size_t roundrobin = 0; roundrobin < num_queues; ++roundrobin) {
          size_t queueidx = 
            (firstown + queues_per_cpu + roundrobin) % num_queues;
          taskqueue_type& queue = task_queues[queueidx];
          queue_locks[queueidx].lock();
          if (!queue.empty()) {
            ret_task = queue.pop().first;
            binary_vertex_tasks.remove(ret_task);
            found = true;
          }
          queue_locks[queueidx].unlock();
          if (found)  break;
        }
      } 
      return found? sched_status::NEWTASK : sched_status::EMPTY;
    } // end of get_next_task


    void add_task(update_task_type task, double priority) {
      const size_t qidx = task.vertex() % task_queues.size();
      queue_locks[qidx].lock();
      if (binary_vertex_tasks.add(task))  terminator.new_job();  
      task_queues[qidx].insert_max(task, priority);     
      queue_locks[qidx].unlock();

    }

    void add_tasks(const std::vector<vertex_id_type> &vertices,
                   update_function_type func,
                   double priority) {
      foreach(vertex_id_type vertex, vertices) {
        add_task(update_task_type(vertex, func), priority);
      }
    }


    void add_task_to_all(update_function_type func, double priority)  {
      for (vertex_id_type vertex = 0; vertex < numvertices; ++vertex){
        add_task(update_task_type(vertex, func), priority);
      }
    }


    bool is_task_scheduled(update_task_type task)  {
      return binary_vertex_tasks.get(task);
    }


    void print() {
      std::cout << "SSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSS" << std::endl;
      std::cout << "Printing task queue sizes: " << std::endl;
      for(size_t i = 0; i < task_queues.size(); ++i) {
        std::cout << task_queues[i].size() << std::endl;
      }
      std::cout << "SSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSS" << std::endl;
        
    }


    void completed_task(size_t cpuid, const update_task_type &task) {
      terminator.completed_job();
    }

    terminator_type& get_terminator() {
      return terminator;
    };

    void set_options(const scheduler_options &opts) { }

    static void print_options_help(std::ostream &out) { };

  private:
    size_t numvertices; /// Remember the number of vertices in the graph
  
    size_t num_queues;
    size_t queues_per_cpu;
  
    std::vector<taskqueue_type> task_queues; /// The actual task queue
    std::vector<mutex> queue_locks;
    std::vector<size_t> lastqueue;

    /// The callbacks pre-created for each cpuid
    std::vector<direct_callback<Graph> > callbacks; 

    // Task set for task pruning
    binary_vertex_task_set<Graph> binary_vertex_tasks;

  
    task_count_termination terminator;
  }; 


} // end of namespace graphlab
#include <graphlab/macros_undef.hpp>

#endif

