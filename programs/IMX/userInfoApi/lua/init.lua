local path_list ={
	"lua/?.lua;",


	"../../../open/lib/?.lua;",
	"../../../open/apply/?.lua;",
	"../../../open/spxonly/?.lua;",
	"../../../open/linkup/?.lua;",
	"../../../open/public/?.lua;",

	"open/?.lua;",
}

package.path = table.concat(path_list) .. package.path
package.cpath = "../../../open/lib/?.so;" .. "open/?.so;" .. package.cpath
