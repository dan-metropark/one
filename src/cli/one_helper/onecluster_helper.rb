# -------------------------------------------------------------------------- #
# Copyright 2002-2012, OpenNebula Project Leads (OpenNebula.org)             #
#                                                                            #
# Licensed under the Apache License, Version 2.0 (the "License"); you may    #
# not use this file except in compliance with the License. You may obtain    #
# a copy of the License at                                                   #
#                                                                            #
# http://www.apache.org/licenses/LICENSE-2.0                                 #
#                                                                            #
# Unless required by applicable law or agreed to in writing, software        #
# distributed under the License is distributed on an "AS IS" BASIS,          #
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.   #
# See the License for the specific language governing permissions and        #
# limitations under the License.                                             #
#--------------------------------------------------------------------------- #

require 'one_helper'

class OneClusterHelper < OpenNebulaHelper::OneHelper
    def self.rname
        "CLUSTER"
    end

    def self.conf_file
        "onecluster.yaml"
    end

    def format_pool(options)
        config_file = self.class.table_conf

        table = CLIHelper::ShowTable.new(config_file, self) do
            column :ID, "ONE identifier for the Cluster", :size=>4 do |d|
                d["ID"]
            end

            column :NAME, "Name of the Cluster", :left, :size=>15 do |d|
                d["NAME"]
            end

            default :ID, :NAME
        end

        table
    end

    private

    def factory(id=nil)
        if id
            OpenNebula::Cluster.new_with_id(id, @client)
        else
            xml=OpenNebula::Cluster.build_xml
            OpenNebula::Cluster.new(xml, @client)
        end
    end

    def factory_pool(user_flag=-2)
        OpenNebula::ClusterPool.new(@client)
    end

    def format_resource(cluster)
        str="%-15s: %-20s"
        str_h1="%-80s"

        CLIHelper.print_header(str_h1 % "CLUSTER #{cluster['ID']} INFORMATION")
        puts str % ["ID",   cluster.id.to_s]
        puts str % ["NAME", cluster.name]
        puts

        CLIHelper.print_header(str_h1 % "HOSTS", false)
        CLIHelper.print_header("%-15s" % ["ID"])
        cluster.host_ids.each do |id|
            puts "%-15s" % [id]
        end
    end
end