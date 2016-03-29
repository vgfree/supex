/**
 * Author       : chenzutao
 * Date         : 2016-01-26
 * Function     : graph.cpp
 *                create graph
 **/

#include <cstdio>
#include <cstdlib>
#include <algorithm>
#include <iostream>
#include <cstring>
#include <string>

#include "graphlab.hpp"
#include "graphlab/macros_def.hpp"
#include "graph.h"

struct vertex_data
{
	float   value;
	float   self_weight;
	vertex_data(float value = 1) : value(value), self_weight(0) {}
};

struct edge_data
{
	float   weight;
	float   old_source_value;
	edge_data(float weight) : weight(weight), old_source_value(0) {}
};

typedef graphlab::graph <vertex_data, edge_data> graph_t;
typedef graphlab::types <graph_t> gl_types;

void create_graph(graph_t &graph)
{
	for (int i = 0; i < 5; i++) {
		graph.add_vertex(vertex_data());
	}

	graph.add_edge(0, 3, edge_data(1));	// road 0 links to road 3 only, so the weight is 1

	graph.add_edge(1, 0, edge_data(0.5));
	graph.add_edge(1, 2, edge_data(0.5));

	graph.add_edge(2, 0, edge_data(1.0 / 3));
	graph.add_edge(2, 1, edge_data(1.0 / 3));
	graph.add_edge(2, 3, edge_data(1.0 / 3));

	graph.add_edge(3, 0, edge_data(0.25));
	graph.add_edge(3, 1, edge_data(0.25));
	graph.add_edge(3, 2, edge_data(0.25));
	graph.add_edge(3, 4, edge_data(0.25));

	graph.add_edge(4, 0, edge_data(0.25));
	graph.add_edge(4, 1, edge_data(0.25));
	graph.add_edge(4, 2, edge_data(0.25));
	graph.add_edge(4, 3, edge_data(0.25));
	// graph.add_edge(4, 4, edge_data(0.2)); // self-edge from road 4 to road 4 must be handled specially
}

#define termination_bound       1e-5
#define damping_factor          0.85

void rr_update(gl_types::iscope &scope, gl_types::icallback &scheduler)
{
	vertex_data &vdata = scope.vertex_data();

	float sum = vdata.value * vdata.self_weight;

	foreach(graphlab::edge_id_t eid, scope.in_edge_ids())
	{
		// get neighbor vertex value
		const vertex_data      &neighbor_vdata = scope.const_neighbor_vertex_data(scope.source(eid));
		double                  neighbor_value = neighbor_vdata.value;

		// get the edge data for the neighbor
		edge_data &edata = scope.edge_data(eid);

		double contribution = edata.weight * neighbor_value;

		sum += contribution;

		edata.old_source_value = neighbor_value;
	}
	sum = (1 - damping_factor) / scope.num_vertices() + damping_factor * sum;
	vdata.value = sum;

	foreach(graphlab::edge_id_t eid, scope.out_edge_ids())
	{
		edge_data &out_edge_data = scope.edge_data(eid);

		double residual = out_edge_data.weight * std::fabs(out_edge_data.old_source_value - vdata.value);

		if (residual > termination_bound) {
			gl_types::update_task task(scope.target(eid), rr_update);
			scheduler.add_task(task, residual);
		}
	}
}

int main(int argc, char **argv)
{
	global_logger().set_log_level(LOG_INFO);
	global_logger().set_log_to_console(true);
	logger(LOG_INFO, "program starting");

	// Setup the parser
	graphlab::command_line_options
		clopts("run rr algorithm.");

	// create graphlab core
	gl_types::core core;

	// parse arguments
	if (!clopts.parse(argc, argv)) {
		std::cout << "Error in parsing argvments ." << std::endl;
		return EXIT_FAILURE;
	}

	core.set_engine_options(clopts);

	// create a synthetic graph
	create_graph(core.graph());

	// schedule all vertices to run the algorithm
	core.add_task_to_all(rr_update, 100);

	double runtime = core.start();

	double norm = 0.0;

	for (graphlab::vertex_id_t vid = 0; vid < core.graph().num_vertices(); vid++) {
		norm += core.graph().vertex_data(vid).value;
	}

	for (graphlab::vertex_id_t vid = 0; vid < 5; vid++) {
		std::cout << "verticeID :" << vid << " rank :" <<
			core.graph().vertex_data(vid).value / norm << std::endl;
	}

	return 0;
}

