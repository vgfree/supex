/**
 * Author       : chenzutao
 * Date         : 2016-01-29
 * Function     : graph.cpp
 *                create roadRank graph
 **/

#include <cstdio>
#include <cstdlib>
#include <algorithm>
#include <iostream>
#include <cstring>
#include <string>
#include <cmath>
#include <vector>
#include <string>

#ifdef USE_CLUSTER
  #include "distributed_graphlab.hpp"
#else
  #include "graphlab.hpp"
#endif

#include "graphlab/macros_def.hpp"
#include "graph.h"
#include "graphlab/graph/disk_graph.hpp"

struct vertex_data
{
	vertex_t        v_value;
	float           self_weight;
	vertex_data(vertex_t v) : v_value(v), self_weight(0) {}

	void            save(graphlab::oarchive &archive) const
	{
		archive << v_value.v_id << v_value.out_edge << v_value.in_edge <<
			v_value.v_attr.color << v_value.v_attr.partition << self_weight;
	}

	void            load(graphlab::iarchive &archive)
	{
		archive >> v_value.v_id >> v_value.out_edge >> v_value.in_edge >>
		v_value.v_attr.color >> v_value.v_attr.partition >> self_weight;
	}
};

SERIALIZABLE_POD(vertex_data);

struct edge_data
{
	edge_t  e_value;
	float   factor;
	float   old_source_value;
	edge_data(edge_t v) : e_value(v), factor(0), old_source_value(0) {}

	void    save(graphlab::oarchive &archive) const
	{
		archive << e_value.e_id << e_value.src_vertice << e_value.dst_vertice <<
			e_value.edge_attr.speed << e_value.edge_attr.limit_speed <<
			e_value.edge_attr.time << e_value.edge_attr.level << e_value.edge_attr.len <<
			e_value.edge_attr.rank << e_value.edge_attr.weight << factor <<
			old_source_value;
	}

	void    load(graphlab::iarchive &archive)
	{
		archive >> e_value.e_id >> e_value.src_vertice >> e_value.dst_vertice >>
		e_value.edge_attr.speed >> e_value.edge_attr.limit_speed >>
		e_value.edge_attr.time >> e_value.edge_attr.level >> e_value.edge_attr.len >>
		e_value.edge_attr.rank >> e_value.edge_attr.weight >> factor >>
		old_source_value;
	}
};

SERIALIZABLE_POD(edge_data);

#ifdef USE_CLUSTER
typedef graphlab::distributed_graph <vertex_data, edge_data> graph_t;
typedef graphlab::distributed_types <graph_t> gl_types;
#else
typedef graphlab::graph <vertex_data, edge_data> graph_t;
typedef graphlab::types <graph_t> gl_types;
#endif

/**
 * [ disk graph file format]
 * edge_id <tab> source_id <tab> target_id <tab> speed <tab> limit_speed <tab> time <tab> level <tab> length <tab>
 * edge_id <tab> source_id <tab> target_id <tab> speed <tab> limit_speed <tab> time <tab> level <tab> length <tab>
 *
 **/
bool construct_graph(const std::string &filename, gl_types::memory_graph &distributed_graph)
{
	std::cout << "Contructing roadRank demo graph from file." << std::endl;
	gl_types::disk_graph dg("RR", 64);
	//        gl_types::memory_graph g;
	std::ifstream fin(filename.c_str());

	if (!fin.good()) {
		logger(LOG_INFO, "read file failed !");
		return false;
	}

	while (fin.good() && !fin.eof()) {
		uint_t  e_id;
		unit_t  source = 0;
		uint_t  target = 0;
		int     speed;
		int     limit_speed;
		int     time;
		int     level;
		int     len;

		fin >> e_id;

		if (!fin.good()) {
			logger(LOG_ERROR, "read edge id from file failed .\n");
			break;
		}

		fin >> source;
		assert(fin.good());

		fin >> target;
		assert(fin.good());

		fin >> speed;
		assert(fin.good());

		fin >> limit_speed;
		assert(fin.good());

		fin >> time;
		assert(fin.good());

		fin >> level;
		assert(fin.good());

		fin >> len;
		assert(fin.good());

		if ((source >= distributed_graph.num_vertices()) || (target >= distributed_graph.num_vertices())) {
			distributed_graph.resize(std::max(source, target) + 1);
			logger(LOG_INFO, "resized graph size to : %u\n", std::max(source, target) + 1);
		}

		if (source != target) {
			edge_attr_t e_attr;
			e_attr.speed = speed;
			e_attr.limit_speed = limit_speed;
			e_attr.time = time;
			e_attr.level = level;
			e_attr.len = len;
			edge_t edata(source, target, e_attr);
			distributed_graph.add_edge(source, target, e_attr);
		}
	}

	std::cout << "Finished loading graph with: " << std::endl;
		<< "\t vertices: " << distributed_graph.num_vertices() << std::endl
		<< "\t deges: " << graph.num_edges() << std::endl;
}

void create_graph(graph_t &graph)
{
	vertex_t v_data[5];

	v_data[0].v_id = 1;
	v_data[0].out_edge = 4;
	v_data[0].in_edge = 0;
	v_data[0].v_attr.color = BLACK;
	v_data[0].v_attr.partition = 0;

	v_data[1].v_id = 2;
	v_data[1].out_edge = 4;
	v_data[1].in_edge = 2;
	v_data[1].v_attr.color = BLACK;
	v_data[1].v_attr.partition = 0;

	v_data[2].v_id = 3;
	v_data[2].out_edge = 1;
	v_data[2].in_edge = 2;
	v_data[2].v_attr.color = BLACK;
	v_data[2].v_attr.partition = 0;

	v_data[3].v_id = 4;
	v_data[3].out_edge = 2;
	v_data[3].in_edge = 1;
	v_data[3].v_attr.color = BLACK;
	v_data[3].v_attr.partition = 0;

	v_data[4].v_id = 5;
	v_data[4].out_edge = 2;
	v_data[4].in_edge = 1;
	v_data[4].v_attr.color = BLACK;
	v_data[4].v_attr.partition = 0;

	for (int i = 0; i < 5; i++) {
		std::cout << "id:" << v_data[i].v_id << ", out_edge:" << v_data[i].out_edge <<
			", in_edge:" << v_data[i].in_edge << ", color:" << v_data[i].v_attr.color <<
			", partition:" << v_data[i].v_attr.partition <<
			std::endl;

		graph.add_vertex(vertex_data(v_data[i]));
	}

	edge_t          edge[10];
	uint_t          src_id[10] = { 1, 1, 1, 1, 2, 4, 3, 5 };
	uint_t          dst_id[10] = { 2, 4, 5, 3, 3, 2, 5, 4 };
	edge_attr_t     e_data[10] = {
		{ 60,  1452582725, 0,  863,  1.0, 0.125 },
		{ 10,  1452582715, 1,  106,  1.0, 0.125 },
		{ 40,  1452582726, 3,  200,  1.0, 0.125 },
		{ 85,  1452582707, 1,  800,  1.0, 0.125 },
		{ 20,  1452582723, 10, 630,  1.0, 0.125 },
		{ 120, 1452582720, 2,  806,  1.0, 0.125 },
		{ 60,  1452582731, 5,  603,  1.0, 0.125 },
		{ 30,  1452582724, 1,  830,  1.0, 0.125 },
		{ 35,  1452582729, 0,  1063, 1.0, 0.125 }
	};

	e_data[2].dst_vertex = 5;

	for (int i = 0; i < 9; i++) {
		edge[i].e_id = i;
		edge[i].src_vertice = src_id[i] - 1;
		edge[i].dst_vertice = dst_id[i] - 1;
		edge[i].edge_attr = e_data[i];
		graph.add_edge(edge[i].src_vertice, edge[i].dst_vertice, edge_data(edge[i]));	// road 0 links to road 3 only, so the weight is 1
	}

	/*
	 *   for(graphlab::vertex_id_t vid=0; vid<graph.num_vertices(); vid++)
	 *   {
	 *        //initialize in edge numbers and out edge numbers on each vertice
	 *   }
	 */
}

#define termination_bound       1e-5
#define damping_factor          0.85
#define natural_e               2.718281828

float get_edge_rank(int speed, int limit_speed, int level)
{
	if (limit_speed != -1) {
		return (float)(speed + 0.0) / limit_speed;
	}

	float   rank = -1.0;
	int     limit = -1;
	switch (level)
	{
		case 0:
		case 10:
			limit = 120;
			break;

		case 1:
		case 2:
			limit = 100;
			break;

		case 3:
			limit = 60;
			break;

		default:
			limit = 40;
			break;
	}

	return (float)(speed + 0.0) / limit;
}

void update(gl_types::iscope &scope, gl_types::icallback &scheduler)
{
	vertex_data &vdata = scope.vertex_data();
	// edge_data &edata = scope.edge_data(eid);
	gl_types::edge_list     in_edge_list = scope.in_edge_ids();
	gl_types::edge_list     out_edge_list = scope.out_edge_ids();
	double                  sum = 0.0;
	time_t                  now;

	time(&now);
	foreach(graphlab::edge_id_t eid, scope.in_edge_ids())
	{
		const edge_data        &neighbor_edata = scope.const_edge_data(eid);
		float                   rank = get_edge_rank(neighbor_edata.e_value.edge_attr.speed,
				neighbor_edata.e_value.edge_attr.limit_speed,
				neighbor_edata.e_value.edge_attr.level);

		int     delta_t = neighbor_edata.e_value.edge_attr.time - now;
		float   factor = log(1 / (1 + pow(natural_e, delta_t))) / log(1 / natural_e);
		double  contribution = rank * neighbor_edata.e_value.edge_attr.weight * factor;

		sum += contribution;

		edge_data &in_edata = scope.edge_data(eid);
		in_edata.old_source_value = neighbor_edata.e_value.edge_attr.rank;
	}

	sum = (1 - damping_factor) / scope.num_vertices() + damping_factor * sum;

	foreach(graphlab::edge_id_t eid, scope.out_edge_ids())
	{
		const size_t            out_num = scope.num_out_neighbors(scope.source(eid));
		const edge_data        &neighbor_edata = scope.const_edge_data(eid);
		edge_data              &edata = scope.edge_data(eid);
		float                   possible = 1.0 / (sum / (out_num + 1));
		double                  rank = possible * get_edge_rank(neighbor_edata.e_value.edge_attr.speed,
				neighbor_edata.e_value.edge_attr.limit_speed,
				neighbor_edata.e_value.edge_attr.level);

		rank = rank * neighbor_edata.e_value.edge_attr.rank;
		edata.e_value.edge_attr.rank = rank;
	}

	foreach(graphlab::edge_id_t eid, scope.out_edge_ids())
	{
		edge_data &out_edge_data = scope.edge_data(eid);

		double residual = out_edge_data.factor * std::fabs(out_edge_data.old_source_value -
				out_edge_data.e_value.edge_attr.rank);

		if (residual > termination_bound) {
			gl_types::update_task task(scope.target(eid), update);
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
		clopts("run roadRank algorithm.");

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
	core.add_task_to_all(update, 100);

	double runtime = core.start();

	for (graphlab::vertex_id_t vid = 0; vid < core.graph().num_vettices(); vid++) {
		// std::cout << "vertex value:" << core.graph().vertex_data(vid).v_value <<
		std::cout << "vertex weight:" << core.graph().vertex_data(vid).self_weight << std::endl;
	}

	for (graphlab::edge_id_t eid = 0; eid < core.graph().num_edges(); eid++) {
		std::cout << "original factor:" << core.graph().edge_data(eid).factor <<
			"old_source_value:" << core.graph().edge_data(eid).old_source_value <<
			"current weight:" << core.graph().edge_data(eid).e_value.edge_attr.weight <<
			"rank:" << core.graph() / edge_data(eid).e_value.edge_attr.rank << std::endl;
	}

	return 0;
}

#include <graphlab/macros_undef.hpp>

