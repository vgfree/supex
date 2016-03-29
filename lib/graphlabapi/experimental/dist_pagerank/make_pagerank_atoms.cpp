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


#include <cassert>
#include <iostream>

#include <graphlab.hpp>



#include "data_structures.hpp"

#include <graphlab/macros_def.hpp>

bool load_graph_from_stdin(local_graph_type& graph) {
  // Loop through file reading each line
  while(std::cin.good() && !std::cin.eof()) {
    if(char(std::cin.peek()) == '#') {
      std::string comment;
      std::getline(std::cin, comment);
      std::cout << "Skipping comment: " << comment << std::endl;
      continue;
    }
    size_t source = 0;
    size_t target = 0;
    std::cin >> source >> target;
    if(!std::cin.good()) break;
    // Ensure that the number of vertices is correct
    if(source >= graph.num_vertices() ||
       target >= graph.num_vertices())
      graph.resize(std::max(source, target) + 1);
    if(source != target) {
      // Add the edge
      const edge_data edata(1.0);
      graph.add_edge(source, target, edata);
    }       
  }
  std::cout 
    << "Finished loading graph with: " << std::endl
    << "\t Vertices: " << graph.num_vertices() << std::endl
    << "\t Edges: " << graph.num_edges() << std::endl;
  std::cout 
    << "Finalizing graph." << std::endl
    << "\t This is required for the locking protocol to function correctly"
    << std::endl;
  graph.finalize();
  std::cout << "Finished finalization!" << std::endl;
  return true;
} // end of load graph





int main(int argc, char** argv) {
  // set the global logger
  global_logger().set_log_level(LOG_INFO);
  global_logger().set_log_to_console(true);

  std::string base_fname("atom");
  std::string index_fname("atom.idx");
  size_t nparts(64);
  graphlab::command_line_options clopts("Make pagerank atoms", true);

  clopts.attach_option("basefname", 
                       &base_fname, 
                       base_fname, 
                       "atom base prefix");
  clopts.attach_option("indexfname", 
                       &index_fname, 
                       index_fname, 
                       "atom index filename");
  clopts.attach_option("nparts", 
                       &nparts, 
                       nparts, 
                       "Number of parts to cut the file into");
  
  if( !clopts.parse(argc, argv) ) { 
    std::cout << "Error parsing command line arguments!"
              << std::endl;
    return EXIT_FAILURE;
  }

  local_graph_type local_graph;
  load_graph_from_stdin(local_graph);

  std::cout << "Coloring graph: " << std::endl;
  local_graph.compute_coloring();

  std::cout << "Partitioning graph: " << std::endl;
  std::vector<graphlab::vertex_id_t> 
    vertex2part(local_graph.num_vertices(), 0);
  graphlab::graph_partitioner::random_partition(local_graph, 
                                                nparts,
                                                vertex2part);

  // local_graph.metis_partition(nparts, vertex2part);
  // graphlab::vertex_id_t maxpart = 0;
  // foreach(graphlab::vertex_id_t part, vertex2part)
  //   maxpart = std::max(maxpart, part);
  // std::cout << "Part: " << maxpart << std::endl;

  std::cout << "Making Atoms Files." << std::endl;
  gl_types::disk_graph dg(base_fname, 
                          nparts, 
                          graphlab::disk_graph_atom_type::WRITE_ONLY_ATOM);
  dg.create_from_graph(local_graph, vertex2part);
  dg.finalize();
  // graph_partition_to_atomindex(local_graph,
  //                              vertex2part,
  //                              base_fname);
  std::cout << "Finished" << std::endl;

  return EXIT_SUCCESS;
}
